---
id: fuuxq
name: How to Add a Command
file_version: 1.0.2
app_version: 0.9.5-5
file_blobs:
  src/mongo/db/s/shardsvr_move_primary_command.cpp: de96165a67211504a63bb6f343773fcd2da212bf
  src/mongo/db/s/migration_destination_manager_legacy_commands.cpp: 419f2144c8ef8ab5c960b92efb0008b1adc6082b
  src/mongo/db/ftdc/ftdc_server.cpp: 2d51d5c202b4699f110f06cf30bca97ead6924fa
  src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp: f1bd1269c86baca9d7047a5b1b9529e63a768bf4
  src/mongo/db/commands.h: 778c62cb2b62c95d6cd24bee97f48c92a04462c4
  src/mongo/db/commands/killoperations_common.h: b9dd9eb79a9f99c1dcc7392ac1769661470d44ee
  src/mongo/db/s/flush_database_cache_updates_command.cpp: a80192e7dd3db40e6676727be2bdeede40e69dec
  src/mongo/s/commands/cluster_merge_chunks_cmd.cpp: 5d826c9c0cfb0bdb30a5fdc48148575f58095bc4
  src/mongo/s/commands/cluster_index_filter_cmd.cpp: f6c5bd37777470ed5422c2b9cf3bded667a26caa
  src/mongo/db/s/wait_for_ongoing_chunk_splits_command.cpp: ff470884b987fe5688779977ecbced2a34339e97
  src/mongo/db/ftdc/ftdc_commands.cpp: aa274c1858e169cfaf1fd00789a488474b5f886d
  src/mongo/db/commands/generic_servers.cpp: 17df971cd858544e2b77b5046a31bfb8a119f7a0
---

In this document, we will learn how to add a new Command to the system.

A Command is {Explain what a Command is and its role in the system}

Some examples of `Command`[<sup id="Z24QAE8">↓</sup>](#f-Z24QAE8)s are `Invocation`[<sup id="Z13x7Wu">↓</sup>](#f-Z13x7Wu), `Invocation`[<sup id="Z1xztO4">↓</sup>](#f-Z1xztO4), `ClusterMergeChunksCommand`[<sup id="ZOckBc">↓</sup>](#f-ZOckBc), and `ClusterIndexFilterCmd`[<sup id="2qrYbs">↓</sup>](#f-2qrYbs). Note: some of these examples inherit indirectly from `Command`[<sup id="Z24QAE8">↓</sup>](#f-Z24QAE8).

## Don't inherit directly from `Command`[<sup id="Z24QAE8">↓</sup>](#f-Z24QAE8) (in most cases)

Most `Command`[<sup id="Z24QAE8">↓</sup>](#f-Z24QAE8)s don't inherit directly from `Command`[<sup id="Z24QAE8">↓</sup>](#f-Z24QAE8), but from one of the following base classes:

*   `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB): Suitable base for {Explain this base class}
    
    *   e.g. `ClusterIndexFilterCmd`[<sup id="2qrYbs">↓</sup>](#f-2qrYbs), `WaitForOngoingChunksSplitsCommand`[<sup id="Zbpg8C">↓</sup>](#f-Zbpg8C), and `GetDiagnosticDataCommand`[<sup id="28BX2D">↓</sup>](#f-28BX2D).
        
*   `TypedCommand`[<sup id="1FpPtN">↓</sup>](#f-1FpPtN): Inherit from this when {Explain this base class}
    
    *   e.g. `Invocation`[<sup id="Z13x7Wu">↓</sup>](#f-Z13x7Wu), `Invocation`[<sup id="Z1xztO4">↓</sup>](#f-Z1xztO4), and `GenericTC`[<sup id="Z1RHu70">↓</sup>](#f-Z1RHu70).
        
*   `ErrmsgCommandDeprecated`[<sup id="2jbdkp">↓</sup>](#f-2jbdkp): Base class of all {Explain this base class}
    
    *   e.g. `ClusterMergeChunksCommand`[<sup id="ZOckBc">↓</sup>](#f-ZOckBc).
        

In this document we demonstrate inheriting from `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB) as it is the most common.

## TL;DR - How to Add a `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB)

1.  Create a new class inheriting from `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB) 
    
    *   Place the file in one of the directories under `📄 src/mongo`, e.g. `MovePrimaryCommand`[<sup id="1de7yd">↓</sup>](#f-1de7yd) is defined in `📄 src/mongo/db/s/shardsvr_move_primary_command.cpp`.
        
2.  Implement `secondaryAllowed`[<sup id="Z1oRpH0">↓</sup>](#f-Z1oRpH0), `supportsWriteConcern`[<sup id="13QziT">↓</sup>](#f-13QziT), `run`[<sup id="Z19c8Wu">↓</sup>](#f-Z19c8Wu), `help`[<sup id="1vN47X">↓</sup>](#f-1vN47X), and `adminOnly`[<sup id="Z1EbmeC">↓</sup>](#f-Z1EbmeC).
    
3.  **Profit** 💰
    

## Example Walkthrough - `MovePrimaryCommand`[<sup id="1de7yd">↓</sup>](#f-1de7yd)

We'll follow the implementation of `MovePrimaryCommand`[<sup id="1de7yd">↓</sup>](#f-1de7yd) for this example.

A `MovePrimaryCommand`[<sup id="1de7yd">↓</sup>](#f-1de7yd) is {Explain what MovePrimaryCommand is and how it works with the Command interface}

## Steps to Adding a new `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB)

### 1\. Inherit from `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB).

All `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB)s are defined under `📄 src/mongo`.

<br/>

We first need to define our class in the relevant file, and inherit from `BasicCommand`[<sup id="Z1LcwlB">↓</sup>](#f-Z1LcwlB):
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/shardsvr_move_primary_command.cpp
```c++
⬜ 41     namespace mongo {
⬜ 42     namespace {
⬜ 43     
🟩 44     class MovePrimaryCommand : public BasicCommand {
⬜ 45     public:
⬜ 46         MovePrimaryCommand() : BasicCommand("_shardsvrMovePrimary") {}
⬜ 47     
```

<br/>

### 2\. Implement `secondaryAllowed`[<sup id="Z1oRpH0">↓</sup>](#f-Z1oRpH0), `supportsWriteConcern`[<sup id="13QziT">↓</sup>](#f-13QziT), `run`[<sup id="Z19c8Wu">↓</sup>](#f-Z19c8Wu), `help`[<sup id="1vN47X">↓</sup>](#f-1vN47X), and `adminOnly`[<sup id="Z1EbmeC">↓</sup>](#f-Z1EbmeC)

Here is how we do it for `MovePrimaryCommand`[<sup id="1de7yd">↓</sup>](#f-1de7yd):

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/shardsvr_move_primary_command.cpp
```c++
⬜ 54             return "should not be calling this directly";
⬜ 55         }
⬜ 56     
🟩 57         AllowedOnSecondary secondaryAllowed(ServiceContext*) const override {
🟩 58             return AllowedOnSecondary::kNever;
🟩 59         }
⬜ 60     
⬜ 61         bool adminOnly() const override {
⬜ 62             return true;
```

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/shardsvr_move_primary_command.cpp
```c++
⬜ 62             return true;
⬜ 63         }
⬜ 64     
🟩 65         bool supportsWriteConcern(const BSONObj& cmd) const override {
🟩 66             return true;
🟩 67         }
⬜ 68     
⬜ 69         Status checkAuthForCommand(Client* client,
⬜ 70                                    const std::string& dbname,
```

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/shardsvr_move_primary_command.cpp
```c++
⬜ 84             return nsElt.str();
⬜ 85         }
⬜ 86     
🟩 87         bool run(OperationContext* opCtx,
🟩 88                  const std::string& dbname_unused,
🟩 89                  const BSONObj& cmdObj,
🟩 90                  BSONObjBuilder& result) override {
🟩 91             uassertStatusOK(ShardingState::get(opCtx)->canAcceptShardedCommands());
🟩 92     
🟩 93             const auto movePrimaryRequest =
🟩 94                 ShardMovePrimary::parse(IDLParserErrorContext("_shardsvrMovePrimary"), cmdObj);
🟩 95             const auto dbname = parseNs("", cmdObj);
🟩 96     
🟩 97             const NamespaceString dbNss(dbname);
🟩 98             const auto toShard = movePrimaryRequest.getTo();
🟩 99     
🟩 100            uassert(
🟩 101                ErrorCodes::InvalidNamespace,
🟩 102                str::stream() << "invalid db name specified: " << dbname,
🟩 103                NamespaceString::validDBName(dbname, NamespaceString::DollarInDbNameBehavior::Allow));
🟩 104    
🟩 105            uassert(ErrorCodes::InvalidOptions,
🟩 106                    str::stream() << "Can't move primary for " << dbname << " database",
🟩 107                    !dbNss.isOnInternalDb());
🟩 108    
🟩 109            uassert(ErrorCodes::InvalidOptions,
🟩 110                    str::stream() << "you have to specify where you want to move it",
🟩 111                    !toShard.empty());
🟩 112    
🟩 113            uassert(
🟩 114                ErrorCodes::InvalidOptions,
🟩 115                str::stream() << "_shardsvrMovePrimary must be called with majority writeConcern, got "
🟩 116                              << cmdObj,
🟩 117                opCtx->getWriteConcern().wMode == WriteConcernOptions::kMajority);
🟩 118    
🟩 119            ON_BLOCK_EXIT(
🟩 120                [opCtx, dbNss] { Grid::get(opCtx)->catalogCache()->purgeDatabase(dbNss.db()); });
🟩 121    
🟩 122            auto coordinatorDoc = MovePrimaryCoordinatorDocument();
🟩 123            coordinatorDoc.setShardingDDLCoordinatorMetadata(
🟩 124                {{dbNss, DDLCoordinatorTypeEnum::kMovePrimary}});
🟩 125            coordinatorDoc.setToShardId(toShard.toString());
🟩 126    
🟩 127            auto service = ShardingDDLCoordinatorService::getService(opCtx);
⬜ 128            auto movePrimaryCoordinator = checked_pointer_cast<MovePrimaryCoordinator>(
⬜ 129                service->getOrCreateInstance(opCtx, coordinatorDoc.toBSON()));
⬜ 130            movePrimaryCoordinator->getCompletionFuture().get(opCtx);
```

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/shardsvr_move_primary_command.cpp
```c++
⬜ 50             return true;
⬜ 51         }
⬜ 52     
🟩 53         std::string help() const override {
🟩 54             return "should not be calling this directly";
🟩 55         }
⬜ 56     
⬜ 57         AllowedOnSecondary secondaryAllowed(ServiceContext*) const override {
⬜ 58             return AllowedOnSecondary::kNever;
```

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/shardsvr_move_primary_command.cpp
```c++
⬜ 58             return AllowedOnSecondary::kNever;
⬜ 59         }
⬜ 60     
🟩 61         bool adminOnly() const override {
🟩 62             return true;
🟩 63         }
⬜ 64     
⬜ 65         bool supportsWriteConcern(const BSONObj& cmd) const override {
⬜ 66             return true;
```

<br/>

## Optionally, these snippets may be helpful

<br/>

Update `📄 src/mongo/db/s/migration_destination_manager_legacy_commands.cpp`
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/migration_destination_manager_legacy_commands.cpp
```c++
⬜ 142    
⬜ 143    } recvChunkStartCmd;
⬜ 144    
🟩 145    class RecvChunkStatusCommand : public BasicCommand {
⬜ 146    public:
⬜ 147        RecvChunkStatusCommand() : BasicCommand("_recvChunkStatus") {}
⬜ 148    
```

<br/>

Update `📄 src/mongo/db/s/migration_destination_manager_legacy_commands.cpp`
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/migration_destination_manager_legacy_commands.cpp
```c++
⬜ 144    
⬜ 145    class RecvChunkStatusCommand : public BasicCommand {
⬜ 146    public:
🟩 147        RecvChunkStatusCommand() : BasicCommand("_recvChunkStatus") {}
⬜ 148    
⬜ 149        bool skipApiVersionCheck() const override {
⬜ 150            // Internal command (server to server).
```

<br/>

Update `📄 src/mongo/db/ftdc/ftdc_server.cpp`
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/ftdc/ftdc_server.cpp
```c++
⬜ 307        // Install file rotation collectors
⬜ 308        // These are collected on each file rotation.
⬜ 309    
🟩 310        // CmdBuildInfo
⬜ 311        controller->addOnRotateCollector(std::make_unique<FTDCSimpleInternalCommandCollector>(
⬜ 312            "buildInfo", "buildInfo", "", BSON("buildInfo" << 1)));
⬜ 313    
```

<br/>

Update `📄 src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp`
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp
```c++
⬜ 124        std::shared_ptr<MigrationChunkClonerSourceLegacy> _chunkCloner;
⬜ 125    };
⬜ 126    
🟩 127    class InitialCloneCommand : public BasicCommand {
⬜ 128    public:
⬜ 129        InitialCloneCommand() : BasicCommand("_migrateClone") {}
⬜ 130    
```

<br/>

Update `📄 src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp`
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp
```c++
⬜ 126    
⬜ 127    class InitialCloneCommand : public BasicCommand {
⬜ 128    public:
🟩 129        InitialCloneCommand() : BasicCommand("_migrateClone") {}
⬜ 130    
⬜ 131        bool skipApiVersionCheck() const override {
⬜ 132            // Internal command (server to server).
```

<br/>

<!-- THIS IS AN AUTOGENERATED SECTION. DO NOT EDIT THIS SECTION DIRECTLY -->
### Swimm Note

<span id="f-Z1EbmeC">adminOnly</span>[^](#Z1EbmeC) - "src/mongo/db/s/shardsvr_move_primary_command.cpp" L61
```c++
    bool adminOnly() const override {
```

<span id="f-Z1LcwlB">BasicCommand</span>[^](#Z1LcwlB) - "src/mongo/db/commands.h" L943
```c
class BasicCommand : public BasicCommandWithReplyBuilderInterface {
```

<span id="f-2qrYbs">ClusterIndexFilterCmd</span>[^](#2qrYbs) - "src/mongo/s/commands/cluster_index_filter_cmd.cpp" L47
```c++
class ClusterIndexFilterCmd : public BasicCommand {
```

<span id="f-ZOckBc">ClusterMergeChunksCommand</span>[^](#ZOckBc) - "src/mongo/s/commands/cluster_merge_chunks_cmd.cpp" L54
```c++
class ClusterMergeChunksCommand : public ErrmsgCommandDeprecated {
```

<span id="f-Z24QAE8">Command</span>[^](#Z24QAE8) - "src/mongo/db/commands.h" L350
```c
class Command {
```

<span id="f-2jbdkp">ErrmsgCommandDeprecated</span>[^](#2jbdkp) - "src/mongo/db/commands.h" L1105
```c
class ErrmsgCommandDeprecated : public BasicCommand {
```

<span id="f-Z1RHu70">GenericTC</span>[^](#Z1RHu70) - "src/mongo/db/commands/generic_servers.cpp" L63
```c++
class GenericTC : public TypedCommand<GenericTC<RequestT, Traits>> {
```

<span id="f-28BX2D">GetDiagnosticDataCommand</span>[^](#28BX2D) - "src/mongo/db/ftdc/ftdc_commands.cpp" L49
```c++
class GetDiagnosticDataCommand final : public BasicCommand {
```

<span id="f-1vN47X">help</span>[^](#1vN47X) - "src/mongo/db/s/shardsvr_move_primary_command.cpp" L53
```c++
    std::string help() const override {
```

<span id="f-Z1xztO4">Invocation</span>[^](#Z1xztO4) - "src/mongo/db/s/flush_database_cache_updates_command.cpp" L84
```c++
    class Invocation final : public TypedCommand<Derived>::InvocationBase {
```

<span id="f-Z13x7Wu">Invocation</span>[^](#Z13x7Wu) - "src/mongo/db/commands/killoperations_common.h" L57
```c
    class Invocation final : public TypedCommand<Derived>::InvocationBase {
```

<span id="f-1de7yd">MovePrimaryCommand</span>[^](#1de7yd) - "src/mongo/db/s/shardsvr_move_primary_command.cpp" L44
```c++
class MovePrimaryCommand : public BasicCommand {
```

<span id="f-Z19c8Wu">run</span>[^](#Z19c8Wu) - "src/mongo/db/s/shardsvr_move_primary_command.cpp" L87
```c++
    bool run(OperationContext* opCtx,
```

<span id="f-Z1oRpH0">secondaryAllowed</span>[^](#Z1oRpH0) - "src/mongo/db/s/shardsvr_move_primary_command.cpp" L57
```c++
    AllowedOnSecondary secondaryAllowed(ServiceContext*) const override {
```

<span id="f-13QziT">supportsWriteConcern</span>[^](#13QziT) - "src/mongo/db/s/shardsvr_move_primary_command.cpp" L65
```c++
    bool supportsWriteConcern(const BSONObj& cmd) const override {
```

<span id="f-1FpPtN">TypedCommand</span>[^](#1FpPtN) - "src/mongo/db/commands.h" L1148
```c
class TypedCommand : public Command {
```

<span id="f-Zbpg8C">WaitForOngoingChunksSplitsCommand</span>[^](#Zbpg8C) - "src/mongo/db/s/wait_for_ongoing_chunk_splits_command.cpp" L40
```c++
class WaitForOngoingChunksSplitsCommand final : public BasicCommand {
```

<br/>

This file was generated by Swimm. [Click here to view it in the app](https://swimm-web-app.web.app/repos/Z2l0aHViJTNBJTNBbW9uZ28lM0ElM0FzYWFyLXN3aW1t/docs/fuuxq).