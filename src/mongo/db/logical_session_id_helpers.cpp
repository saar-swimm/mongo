/**
 *    Copyright (C) 2018-present MongoDB, Inc.
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

#include "mongo/db/logical_session_id_helpers.h"

#include "mongo/db/auth/authorization_session.h"
#include "mongo/db/auth/user.h"
#include "mongo/db/auth/user_name.h"
#include "mongo/db/commands/feature_compatibility_version_documentation.h"
#include "mongo/db/logical_session_cache.h"
#include "mongo/db/operation_context.h"

namespace mongo {

/**
 * This is a safe hash that will not collide with a username because all full usernames include an
 * '@' character.
 */
const auto kNoAuthDigest = SHA256Block::computeHash(reinterpret_cast<const uint8_t*>(""), 0);

SHA256Block getLogicalSessionUserDigestForLoggedInUser(const OperationContext* opCtx) {
    auto client = opCtx->getClient();
    ServiceContext* serviceContext = client->getServiceContext();

    if (AuthorizationManager::get(serviceContext)->isAuthEnabled()) {
        UserName userName;

        const auto user = AuthorizationSession::get(client)->getSingleUser();
        invariant(user);

        uassert(ErrorCodes::BadValue,
                "Username too long to use with logical sessions",
                user->getName().getDisplayNameLength() < kMaximumUserNameLengthForLogicalSessions);

        return user->getDigest();
    } else {
        return kNoAuthDigest;
    }
}

SHA256Block getLogicalSessionUserDigestFor(StringData user, StringData db) {
    if (user.empty() && db.empty()) {
        return kNoAuthDigest;
    }
    const UserName un(user, db);
    auto fn = un.getDisplayName();
    return SHA256Block::computeHash({ConstDataRange(fn.c_str(), fn.size())});
}

boost::optional<LogicalSessionId> getParentSessionId(const LogicalSessionId& sessionId) {
    if (sessionId.getTxnUUID()) {
        return LogicalSessionId{sessionId.getId(), sessionId.getUid()};
    }
    return boost::none;
}

LogicalSessionId castToParentSessionId(const LogicalSessionId& sessionId) {
    if (auto parentSessionId = getParentSessionId(sessionId)) {
        return *parentSessionId;
    }
    return sessionId;
}

bool isInternalSessionForRetryableWrite(const LogicalSessionId& sessionId) {
    return sessionId.getTxnNumber().has_value();
}

LogicalSessionId makeLogicalSessionIdWithTxnNumberAndUUID(const LogicalSessionId& parentLsid,
                                                          TxnNumber txnNumber) {
    auto lsid = LogicalSessionId(parentLsid.getId(), parentLsid.getUid());
    lsid.getInternalSessionFields().setTxnNumber(txnNumber);
    lsid.getInternalSessionFields().setTxnUUID(UUID::gen());
    return lsid;
}

LogicalSessionId makeLogicalSessionIdWithTxnUUID(const LogicalSessionId& parentLsid) {
    auto lsid = LogicalSessionId(parentLsid.getId(), parentLsid.getUid());
    lsid.getInternalSessionFields().setTxnUUID(UUID::gen());
    return lsid;
}

LogicalSessionId makeLogicalSessionId(const LogicalSessionFromClient& fromClient,
                                      OperationContext* opCtx,
                                      std::initializer_list<Privilege> allowSpoof) {
    uassert(ErrorCodes::InvalidOptions,
            "Cannot specify txnNumber in lsid without specifying txnUUID",
            !fromClient.getTxnNumber() || fromClient.getTxnUUID());

    LogicalSessionId lsid;

    lsid.setId(fromClient.getId());
    lsid.getInternalSessionFields().setTxnNumber(fromClient.getTxnNumber());
    lsid.getInternalSessionFields().setTxnUUID(fromClient.getTxnUUID());

    if (fromClient.getUid()) {
        auto authSession = AuthorizationSession::get(opCtx->getClient());

        uassert(ErrorCodes::Unauthorized,
                "Unauthorized to set user digest in LogicalSessionId",
                std::any_of(allowSpoof.begin(),
                            allowSpoof.end(),
                            [&](const auto& priv) {
                                return authSession->isAuthorizedForPrivilege(priv);
                            }) ||
                    authSession->isAuthorizedForPrivilege(Privilege(
                        ResourcePattern::forClusterResource(), ActionType::impersonate)) ||
                    getLogicalSessionUserDigestForLoggedInUser(opCtx) == fromClient.getUid());

        lsid.setUid(*fromClient.getUid());
    } else {
        lsid.setUid(getLogicalSessionUserDigestForLoggedInUser(opCtx));
    }

    return lsid;
}

LogicalSessionId makeLogicalSessionId(OperationContext* opCtx) {
    LogicalSessionId id{};

    id.setId(UUID::gen());
    id.setUid(getLogicalSessionUserDigestForLoggedInUser(opCtx));

    return id;
}

LogicalSessionId makeSystemLogicalSessionId() {
    LogicalSessionId id{};

    id.setId(UUID::gen());
    id.setUid((*internalSecurity.getUser())->getDigest());

    return id;
}

LogicalSessionRecord makeLogicalSessionRecord(OperationContext* opCtx, Date_t lastUse) {
    LogicalSessionId id{};
    LogicalSessionRecord lsr{};

    auto client = opCtx->getClient();
    ServiceContext* serviceContext = client->getServiceContext();
    if (AuthorizationManager::get(serviceContext)->isAuthEnabled()) {
        auto user = AuthorizationSession::get(client)->getSingleUser();
        invariant(user);

        id.setUid(user->getDigest());
        lsr.setUser(StringData(user->getName().getDisplayName()));
    } else {
        id.setUid(kNoAuthDigest);
    }

    id.setId(UUID::gen());

    lsr.setId(id);
    lsr.setLastUse(lastUse);

    return lsr;
}

LogicalSessionRecord makeLogicalSessionRecord(const LogicalSessionId& lsid, Date_t lastUse) {
    LogicalSessionId id{};
    LogicalSessionRecord lsr{};

    id.setId(lsid.getId());
    id.setUid(lsid.getUid());

    lsr.setId(id);
    lsr.setLastUse(lastUse);

    return lsr;
}

LogicalSessionRecord makeLogicalSessionRecord(OperationContext* opCtx,
                                              const LogicalSessionId& lsid,
                                              Date_t lastUse) {
    auto lsr = makeLogicalSessionRecord(lsid, lastUse);

    auto client = opCtx->getClient();
    ServiceContext* serviceContext = client->getServiceContext();
    if (AuthorizationManager::get(serviceContext)->isAuthEnabled()) {
        auto user = AuthorizationSession::get(client)->getSingleUser();
        invariant(user);

        if (user->getDigest() == lsid.getUid()) {
            lsr.setUser(StringData(user->getName().getDisplayName()));
        }
    }

    return lsr;
}


LogicalSessionToClient makeLogicalSessionToClient(const LogicalSessionId& lsid) {
    LogicalSessionIdToClient lsitc;
    lsitc.setId(lsid.getId());

    LogicalSessionToClient id;

    id.setId(lsitc);
    id.setTimeoutMinutes(localLogicalSessionTimeoutMinutes);

    return id;
};

LogicalSessionIdSet makeLogicalSessionIds(const std::vector<LogicalSessionFromClient>& sessions,
                                          OperationContext* opCtx,
                                          std::initializer_list<Privilege> allowSpoof) {
    LogicalSessionIdSet lsids;
    lsids.reserve(sessions.size());
    for (auto&& session : sessions) {
        lsids.emplace(makeLogicalSessionId(session, opCtx, allowSpoof));
    }

    return lsids;
}

namespace logical_session_id_helpers {

void serializeLsidAndTxnNumber(OperationContext* opCtx, BSONObjBuilder* builder) {
    OperationSessionInfo sessionInfo;
    sessionInfo.setSessionId(opCtx->getLogicalSessionId());
    sessionInfo.setTxnNumber(opCtx->getTxnNumber());
    sessionInfo.serialize(builder);
}

}  // namespace logical_session_id_helpers
}  // namespace mongo