// Basic sanity check of log component helpers

(function(db) {
"use strict";
var mongo = db.getMongo();

// Get current log component settings. We will reset to these later.
var originalSettings =
    assert.commandWorked(db.adminCommand({getParameter: 1, logComponentVerbosity: 1}))
        .logComponentVerbosity;

// getLogComponents
var components1 = mongo.getLogComponents();
assert.docEq(components1, originalSettings);

// getLogComponents via db
var components2 = db.getLogComponents();
assert.docEq(components2, originalSettings);

// setLogLevel - default component
mongo.setLogLevel(2);
assert.eq(mongo.getLogComponents().verbosity, 2);

db.setLogLevel(0);
assert.eq(mongo.getLogComponents().verbosity, 0);

// setLogLevel - valid log component
mongo.setLogLevel(2, "storage.journal");
assert.eq(mongo.getLogComponents().storage.journal.verbosity, 2);

db.setLogLevel(1, "storage.journal");
assert.eq(mongo.getLogComponents().storage.journal.verbosity, 1);

// setLogLevel - invalid argument
assert.throws(function() {
    mongo.setLogLevel(2, 24);
});
assert.throws(function() {
    db.setLogLevel(2, ["array", "not.allowed"]);
});

// Restore originalSettings
// We need to remove the verbosity settings related to WiredTiger as they cannot be set at runtime.
delete originalSettings["storage"]["wt"];
assert.commandWorked(db.adminCommand({setParameter: 1, logComponentVerbosity: originalSettings}));
}(db));
