/**
 * Tests createIndexes with the 'clustered' option on a replicated collection.
 *
 * @tags: [
 *   requires_fcv_52,
 *   # Does not support sharding
 *   assumes_against_mongod_not_mongos,
 *   assumes_unsharded_collection,
 * ]
 */
(function() {
"use strict";

load("jstests/libs/clustered_collections/clustered_collection_util.js");
load("jstests/libs/clustered_collections/clustered_collection_create_index_clustered_common.js");

const conn = MongoRunner.runMongod();

if (ClusteredCollectionUtil.areClusteredIndexesEnabled(conn) == false) {
    jsTestLog('Skipping test because the clustered indexes feature flag is disabled');
    MongoRunner.stopMongod(conn);
    return;
}

const nonReplicatedDB = conn.getDB("local");
const collName = "coll";
const nonReplicatedColl = nonReplicatedDB[collName];

CreateIndexesClusteredTest.runBaseTests(nonReplicatedDB, collName);

MongoRunner.stopMongod(conn);
})();
