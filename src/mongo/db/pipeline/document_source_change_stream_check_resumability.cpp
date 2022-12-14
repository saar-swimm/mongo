/**
 *    Copyright (C) 2021-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/curop.h"
#include "mongo/db/pipeline/document_source_change_stream_check_resumability.h"
#include "mongo/db/query/query_feature_flags_gen.h"
#include "mongo/db/repl/oplog_entry.h"

using boost::intrusive_ptr;
namespace mongo {
namespace {

REGISTER_INTERNAL_DOCUMENT_SOURCE(_internalChangeStreamCheckResumability,
                                  LiteParsedDocumentSourceChangeStreamInternal::parse,
                                  DocumentSourceChangeStreamCheckResumability::createFromBson,
                                  true);

}  // namespace

// Returns ResumeStatus::kFoundToken if the document retrieved from the resumed pipeline satisfies
// the client's resume token, ResumeStatus::kCheckNextDoc if it is older than the client's token,
// and ResumeToken::kCannotResume if it is more recent than the client's resume token (indicating
// that we will never see the token). If the resume token's documentKey contains only the _id field
// while the pipeline documentKey contains additional fields, then the collection has become sharded
// since the resume token was generated. In that case, we relax the requirements such that only the
// timestamp, version, txnOpIndex, UUID and documentKey._id need match. This remains correct, since
// the only circumstances under which the resume token omits the shard key is if it was generated
// either (1) before the collection was sharded, (2) after the collection was sharded but before the
// primary shard became aware of that fact, implying that it was before the first chunk moved off
// the shard, or (3) by a malicious client who has constructed their own resume token. In the first
// two cases, we can be guaranteed that the _id is unique and the stream can therefore be resumed
// seamlessly; in the third case, the worst that can happen is that some entries are missed or
// duplicated. Note that the simple collation is used to compare the resume tokens, and that we
// purposefully avoid the user's requested collation if present.
DocumentSourceChangeStreamCheckResumability::ResumeStatus
DocumentSourceChangeStreamCheckResumability::compareAgainstClientResumeToken(
    const intrusive_ptr<ExpressionContext>& expCtx,
    const Document& documentFromResumedStream,
    const ResumeTokenData& tokenDataFromClient) {
    // Parse the stream doc into comprehensible ResumeTokenData.
    auto tokenDataFromResumedStream =
        ResumeToken::parse(documentFromResumedStream["_id"].getDocument()).getData();

    // We start the resume with a $gte query on the timestamp, so we never expect it to be lower
    // than our resume token's timestamp.
    invariant(tokenDataFromResumedStream.clusterTime >= tokenDataFromClient.clusterTime);

    // If the clusterTime differs from the client's token, this stream cannot be resumed.
    if (tokenDataFromResumedStream.clusterTime != tokenDataFromClient.clusterTime) {
        return ResumeStatus::kSurpassedToken;
    }

    // If the tokenType exceeds the client token's type, then we have passed the resume token point.
    // This can happen if the client resumes from a synthetic 'high water mark' token from another
    // shard which happens to have the same clusterTime as an actual change on this shard.
    if (tokenDataFromResumedStream.tokenType != tokenDataFromClient.tokenType) {
        return tokenDataFromResumedStream.tokenType > tokenDataFromClient.tokenType
            ? ResumeStatus::kSurpassedToken
            : ResumeStatus::kCheckNextDoc;
    }

    // If the document's 'txnIndex' sorts before that of the client token, we must keep looking.
    if (tokenDataFromResumedStream.txnOpIndex < tokenDataFromClient.txnOpIndex) {
        return ResumeStatus::kCheckNextDoc;
    } else if (tokenDataFromResumedStream.txnOpIndex > tokenDataFromClient.txnOpIndex) {
        // This could happen if the client provided a txnOpIndex of 0, yet the 0th document in the
        // applyOps was irrelevant (meaning it was an operation on a collection or DB not being
        // watched). Signal that we have read beyond the resume token.
        return ResumeStatus::kSurpassedToken;
    }

    // If 'fromInvalidate' exceeds the client's token value, then we have passed the resume point.
    if (tokenDataFromResumedStream.fromInvalidate != tokenDataFromClient.fromInvalidate) {
        return tokenDataFromResumedStream.fromInvalidate ? ResumeStatus::kSurpassedToken
                                                         : ResumeStatus::kCheckNextDoc;
    }

    // It is acceptable for the stream UUID to differ from the client's, if this is a whole-database
    // or cluster-wide stream and we are comparing operations from different shards at the same
    // clusterTime. If the stream UUID sorts after the client's, however, then the stream is not
    // resumable; we are past the point in the stream where the token should have appeared.
    if (tokenDataFromResumedStream.uuid != tokenDataFromClient.uuid) {
        // If we are running on a replica set deployment, we don't ever expect to see identical time
        // stamps and txnOpIndex but differing UUIDs, and we reject the resume attempt at once.
        if (!expCtx->inMongos && !expCtx->needsMerge) {
            return ResumeStatus::kSurpassedToken;
        }
        // Otherwise, return a ResumeStatus based on the sort-order of the client and stream UUIDs.
        return tokenDataFromResumedStream.uuid > tokenDataFromClient.uuid
            ? ResumeStatus::kSurpassedToken
            : ResumeStatus::kCheckNextDoc;
    }

    // If all the fields match exactly, then we have found the token.
    if (ValueComparator::kInstance.evaluate(tokenDataFromResumedStream.documentKey ==
                                            tokenDataFromClient.documentKey)) {
        return ResumeStatus::kFoundToken;
    }

    // At this point, we know that the tokens differ only by documentKey. The status we return will
    // depend on whether the stream token is logically before or after the client token. If the
    // latter, then we will never see the resume token and the stream cannot be resumed. However,
    // before we can return this value, we need to check the possibility that the resumed stream is
    // on a sharded collection and the client token is from before the collection was sharded.
    const auto defaultResumeStatus =
        ValueComparator::kInstance.evaluate(tokenDataFromResumedStream.documentKey >
                                            tokenDataFromClient.documentKey)
        ? ResumeStatus::kSurpassedToken
        : ResumeStatus::kCheckNextDoc;

    // If we're not running in a sharded context, we don't need to proceed any further.
    if (!expCtx->needsMerge && !expCtx->inMongos) {
        return defaultResumeStatus;
    }

    // If we reach here, we still need to check the possibility that the collection has become
    // sharded in the time since the client's resume token was generated. If so, then the client
    // token will only have an _id field, while the token from the new pipeline may have additional
    // shard key fields.

    // We expect the documentKey to be an object in both the client and stream tokens. If either is
    // not, then we cannot compare the embedded _id values in each, and so the stream token does not
    // satisfy the client token.
    if (tokenDataFromClient.documentKey.getType() != BSONType::Object ||
        tokenDataFromResumedStream.documentKey.getType() != BSONType::Object) {
        return defaultResumeStatus;
    }

    auto documentKeyFromResumedStream = tokenDataFromResumedStream.documentKey.getDocument();
    auto documentKeyFromClient = tokenDataFromClient.documentKey.getDocument();

    // In order for the relaxed comparison to be applicable, the client token must have a single _id
    // field, and the resumed stream token must have additional fields beyond _id.
    if (!(documentKeyFromClient.computeSize() == 1 &&
          documentKeyFromResumedStream.computeSize() > 1)) {
        return defaultResumeStatus;
    }

    // If the resume token's documentKey only contains the _id field while the pipeline's
    // documentKey contains additional fields, we require only that the _ids match.
    return (!documentKeyFromClient["_id"].missing() &&
                    ValueComparator::kInstance.evaluate(documentKeyFromResumedStream["_id"] ==
                                                        documentKeyFromClient["_id"])
                ? ResumeStatus::kFoundToken
                : defaultResumeStatus);
}

DocumentSourceChangeStreamCheckResumability::DocumentSourceChangeStreamCheckResumability(
    const intrusive_ptr<ExpressionContext>& expCtx, ResumeTokenData token)
    : DocumentSource(getSourceName(), expCtx), _tokenFromClient(std::move(token)) {}

intrusive_ptr<DocumentSourceChangeStreamCheckResumability>
DocumentSourceChangeStreamCheckResumability::create(const intrusive_ptr<ExpressionContext>& expCtx,
                                                    const DocumentSourceChangeStreamSpec& spec) {
    auto resumeToken = DocumentSourceChangeStream::resolveResumeTokenFromSpec(spec);
    return new DocumentSourceChangeStreamCheckResumability(expCtx, std::move(resumeToken));
}

intrusive_ptr<DocumentSourceChangeStreamCheckResumability>
DocumentSourceChangeStreamCheckResumability::createFromBson(
    BSONElement spec, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
    uassert(5467603,
            str::stream() << "the '" << kStageName << "' object spec must be an object",
            spec.type() == Object);

    auto parsed = DocumentSourceChangeStreamCheckResumabilitySpec::parse(
        IDLParserErrorContext("DocumentSourceChangeStreamCheckResumabilitySpec"),
        spec.embeddedObject());
    return new DocumentSourceChangeStreamCheckResumability(expCtx,
                                                           parsed.getResumeToken().getData());
}

const char* DocumentSourceChangeStreamCheckResumability::getSourceName() const {
    return kStageName.rawData();
}

DocumentSource::GetNextResult DocumentSourceChangeStreamCheckResumability::doGetNext() {
    if (_resumeStatus == ResumeStatus::kSurpassedToken) {
        return pSource->getNext();
    }

    while (_resumeStatus != ResumeStatus::kSurpassedToken) {
        // The underlying oplog scan will throw OplogQueryMinTsMissing if the minTs in the change
        // stream filter has fallen off the oplog. Catch this and throw a more explanatory error.
        auto nextInput = [this]() {
            try {
                return pSource->getNext();
            } catch (const ExceptionFor<ErrorCodes::OplogQueryMinTsMissing>&) {
                uasserted(ErrorCodes::ChangeStreamHistoryLost,
                          "Resume of change stream was not possible, as the resume point may no "
                          "longer be in the oplog.");
            }
        }();

        // If we hit EOF, return it immediately.
        if (!nextInput.isAdvanced()) {
            return nextInput;
        }

        // Determine whether the current event sorts before, equal to or after the resume token.
        _resumeStatus =
            DocumentSourceChangeStreamCheckResumability::compareAgainstClientResumeToken(
                pExpCtx, nextInput.getDocument(), _tokenFromClient);
        switch (_resumeStatus) {
            case ResumeStatus::kCheckNextDoc:
                // If the result was kCheckNextDoc, we are resumable but must swallow this event.
                continue;
            case ResumeStatus::kSurpassedToken:
                // In this case the resume token wasn't found; it may be on another shard. However,
                // since the oplog scan did not throw, we know that we are resumable. Fall through
                // into the following case and return the document.
            case ResumeStatus::kFoundToken:
                // We found the actual token! Return the doc so DSEnsureResumeTokenPresent sees it.
                return nextInput;
        }
    }
    MONGO_UNREACHABLE;
}

Value DocumentSourceChangeStreamCheckResumability::serialize(
    boost::optional<ExplainOptions::Verbosity> explain) const {
    return explain
        ? Value(DOC(DocumentSourceChangeStream::kStageName
                    << DOC("stage"
                           << "internalCheckResumability"_sd
                           << "resumeToken" << ResumeToken(_tokenFromClient).toDocument())))
        : Value(Document{
              {DocumentSourceChangeStreamCheckResumability::kStageName,
               DocumentSourceChangeStreamCheckResumabilitySpec(ResumeToken(_tokenFromClient))
                   .toBSON()}});
}

}  // namespace mongo
