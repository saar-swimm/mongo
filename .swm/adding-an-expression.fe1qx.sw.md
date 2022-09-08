---
id: fe1qx
name: Adding an Expression
file_version: 1.0.2
app_version: 0.9.5-5
file_blobs:
  src/mongo/db/pipeline/expression.h: e2caabf66053343773418b89ca51c8ff19a320b6
  src/mongo/db/pipeline/expression.cpp: a41f610db049b2583bb82ae275c729635c61ec64
  src/mongo/db/pipeline/expression_visitor.h: 7961c549c16fc9ccb3f2191005f38db17e28146a
  src/mongo/db/query/sbe_stage_builder_expression.cpp: 5720bbb7ec919a01d592f15aeab57afe6651253b
  src/mongo/db/cst/cst_pipeline_translation.cpp: 36244d87fa375dcff4347605edf321d42f0b9be1
---

In this document, we will learn how to add a new Expression to the system.

An Expression is {Explain what a Expression is and its role in the system}

Some examples of `Expression`[<sup id="1YUCQJ">↓</sup>](#f-1YUCQJ)s are `ExpressionConstant`[<sup id="ZDExwG">↓</sup>](#f-ZDExwG), `ExpressionCompare`[<sup id="1zjaOt">↓</sup>](#f-1zjaOt), `ExpressionFromAccumulator`[<sup id="Zvf9D6">↓</sup>](#f-Zvf9D6), and `ExpressionObject`[<sup id="Z6B4Yv">↓</sup>](#f-Z6B4Yv). Note: some of these examples inherit indirectly from `Expression`[<sup id="1YUCQJ">↓</sup>](#f-1YUCQJ).

## Which base class to choose? 🤔

*   `Expression`[<sup id="1YUCQJ">↓</sup>](#f-1YUCQJ): Base class of all {Explain this base class}
    
    *   e.g. `ExpressionConstant`[<sup id="ZDExwG">↓</sup>](#f-ZDExwG), `ExpressionObject`[<sup id="Z6B4Yv">↓</sup>](#f-Z6B4Yv), and `ExpressionMeta`[<sup id="KogNe">↓</sup>](#f-KogNe).
        
*   `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH): Suitable base for {Explain this base class}
    
    *   e.g. `ExpressionCompare`[<sup id="1zjaOt">↓</sup>](#f-1zjaOt), `ExpressionSubtract`[<sup id="rz60e">↓</sup>](#f-rz60e), and `ExpressionType`[<sup id="ZmhFFx">↓</sup>](#f-ZmhFFx).
        
*   `DateExpressionAcceptingTimeZone`[<sup id="24yrKg">↓</sup>](#f-24yrKg): Inherit from this when {Explain this base class}
    
    *   e.g. `ExpressionDayOfMonth`[<sup id="ZrN6LY">↓</sup>](#f-ZrN6LY).
        
*   `ExpressionVariadic`[<sup id="13deN9">↓</sup>](#f-13deN9): Suitable base for {Explain this base class}
    
    *   e.g. `ExpressionFromAccumulator`[<sup id="Zvf9D6">↓</sup>](#f-Zvf9D6).
        

In this document we demonstrate inheriting from `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH) as it is the most common.

## TL;DR - How to Add a `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH)

1.  Create a new class inheriting from `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH)  in `📄 src/mongo/db/pipeline/expression.h`.
    
2.  Implement `evaluate`[<sup id="Z1rHSsm">↓</sup>](#f-Z1rHSsm), `getOpName`[<sup id="15MGit">↓</sup>](#f-15MGit), and `acceptVisitor`[<sup id="hFSg">↓</sup>](#f-hFSg).
    
3.  Update relevant files with the new class (see below).
    
4.  **Profit** 💰
    

## Example Walkthrough - `ExpressionTsSecond`[<sup id="PRwTQ">↓</sup>](#f-PRwTQ)

We'll follow the implementation of `ExpressionTsSecond`[<sup id="PRwTQ">↓</sup>](#f-PRwTQ) for this example.

An `ExpressionTsSecond`[<sup id="PRwTQ">↓</sup>](#f-PRwTQ) is {Explain what ExpressionTsSecond is and how it works with the Expression interface}

## Steps to Adding a new `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH)

### 1\. Inherit from `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH).

All `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH)s are defined in `📄 src/mongo/db/pipeline/expression.h`.

<br/>

We first need to define our class in the relevant file, and inherit from `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH):
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.h
```c
⬜ 4162       boost::intrusive_ptr<Expression>& _value;
⬜ 4163   };
⬜ 4164   
🟩 4165   class ExpressionTsSecond final : public ExpressionFixedArity<ExpressionTsSecond, 1> {
⬜ 4166   public:
⬜ 4167       static constexpr const char* const opName = "$tsSecond";
⬜ 4168   
```

<br/>

> **Note**: the class name should start with "Expression".

### 2\. Implement `evaluate`[<sup id="Z1rHSsm">↓</sup>](#f-Z1rHSsm), `getOpName`[<sup id="15MGit">↓</sup>](#f-15MGit), and `acceptVisitor`[<sup id="hFSg">↓</sup>](#f-hFSg)

Here is how we do it for `ExpressionTsSecond`[<sup id="PRwTQ">↓</sup>](#f-PRwTQ):

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.cpp
```c++
⬜ 7841   
⬜ 7842   /* ------------------------- ExpressionTsSecond ----------------------------- */
⬜ 7843   
🟩 7844   Value ExpressionTsSecond::evaluate(const Document& root, Variables* variables) const {
🟩 7845       const Value operand = _children[0]->evaluate(root, variables);
🟩 7846   
🟩 7847       if (operand.nullish()) {
🟩 7848           return Value(BSONNULL);
🟩 7849       }
🟩 7850   
🟩 7851       uassert(5687301,
🟩 7852               str::stream() << " Argument to " << opName << " must be a timestamp, but is "
🟩 7853                             << typeName(operand.getType()),
🟩 7854               operand.getType() == BSONType::bsonTimestamp);
🟩 7855   
🟩 7856       return Value(static_cast<long long>(operand.getTimestamp().getSecs()));
🟩 7857   }
⬜ 7858   
⬜ 7859   REGISTER_EXPRESSION_WITH_MIN_VERSION(
⬜ 7860       tsSecond,
```

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.h
```c
⬜ 4174   
⬜ 4175       Value evaluate(const Document& root, Variables* variables) const final;
⬜ 4176   
🟩 4177       const char* getOpName() const final {
🟩 4178           return opName;
🟩 4179       }
⬜ 4180   
⬜ 4181       void acceptVisitor(ExpressionMutableVisitor* visitor) final {
⬜ 4182           return visitor->visit(this);
```

<br/>



<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.h
```c
⬜ 4182           return visitor->visit(this);
⬜ 4183       }
⬜ 4184   
🟩 4185       void acceptVisitor(ExpressionConstVisitor* visitor) const final {
🟩 4186           return visitor->visit(this);
🟩 4187       }
⬜ 4188   };
⬜ 4189   
⬜ 4190   class ExpressionTsIncrement final : public ExpressionFixedArity<ExpressionTsIncrement, 1> {
```

<br/>

## Update additional files with the new class

Every time we add new `ExpressionFixedArity`[<sup id="Z2M4eH">↓</sup>](#f-Z2M4eH)s, we reference them in a few locations.

We will still look at `ExpressionTsSecond`[<sup id="PRwTQ">↓</sup>](#f-PRwTQ) as our example.

<br/>

3\. Don't forget to add the new class to `📄 src/mongo/db/pipeline/expression.cpp`, as we do with `ExpressionTsSecond`[<sup id="PRwTQ">↓</sup>](#f-PRwTQ) here:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.cpp
```c++
⬜ 7839                                       {"value"_sd, _value->serialize(explain)}}}});
⬜ 7840   }
⬜ 7841   
🟩 7842   /* ------------------------- ExpressionTsSecond ----------------------------- */
⬜ 7843   
🟩 7844   Value ExpressionTsSecond::evaluate(const Document& root, Variables* variables) const {
⬜ 7845       const Value operand = _children[0]->evaluate(root, variables);
⬜ 7846   
⬜ 7847       if (operand.nullish()) {
```

<br/>

In addition, in the same file:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.cpp
```c++
⬜ 7858   
⬜ 7859   REGISTER_EXPRESSION_WITH_MIN_VERSION(
⬜ 7860       tsSecond,
🟩 7861       ExpressionTsSecond::parse,
⬜ 7862       AllowedWithApiStrict::kNeverInVersion1,
⬜ 7863       AllowedWithClientType::kAny,
⬜ 7864       multiversion::FeatureCompatibilityVersion::kFullyDowngradedTo_5_0);
```

<br/>

3\. We modify `📄 src/mongo/db/pipeline/expression.h`, like so:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression.h
```c
⬜ 4162       boost::intrusive_ptr<Expression>& _value;
⬜ 4163   };
⬜ 4164   
🟩 4165   class ExpressionTsSecond final : public ExpressionFixedArity<ExpressionTsSecond, 1> {
⬜ 4166   public:
⬜ 4167       static constexpr const char* const opName = "$tsSecond";
⬜ 4168   
🟩 4169       explicit ExpressionTsSecond(ExpressionContext* const expCtx)
🟩 4170           : ExpressionFixedArity<ExpressionTsSecond, 1>(expCtx) {}
⬜ 4171   
🟩 4172       ExpressionTsSecond(ExpressionContext* const expCtx, ExpressionVector&& children)
🟩 4173           : ExpressionFixedArity<ExpressionTsSecond, 1>(expCtx, std::move(children)) {}
⬜ 4174   
⬜ 4175       Value evaluate(const Document& root, Variables* variables) const final;
⬜ 4176   
```

<br/>

3\. Add the new class to `📄 src/mongo/db/pipeline/expression_visitor.h`, as seen here:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression_visitor.h
```c
⬜ 175    class AccumulatorSum;
⬜ 176    class AccumulatorMergeObjects;
⬜ 177    
🟩 178    class ExpressionTsSecond;
⬜ 179    class ExpressionTsIncrement;
⬜ 180    
⬜ 181    template <typename AccumulatorState>
```

<br/>

Additionally in the same file:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression_visitor.h
```c
⬜ 358        virtual void visit(expression_walker::MaybeConstPtr<IsConst, ExpressionDateSubtract>) = 0;
⬜ 359        virtual void visit(expression_walker::MaybeConstPtr<IsConst, ExpressionGetField>) = 0;
⬜ 360        virtual void visit(expression_walker::MaybeConstPtr<IsConst, ExpressionSetField>) = 0;
🟩 361        virtual void visit(expression_walker::MaybeConstPtr<IsConst, ExpressionTsSecond>) = 0;
⬜ 362        virtual void visit(expression_walker::MaybeConstPtr<IsConst, ExpressionTsIncrement>) = 0;
⬜ 363    };
⬜ 364    
```

<br/>

Also notice in the same file:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/pipeline/expression_visitor.h
```c
⬜ 516        void visit(const ExpressionDateSubtract*) override {}
⬜ 517        void visit(const ExpressionGetField*) override {}
⬜ 518        void visit(const ExpressionSetField*) override {}
🟩 519        void visit(const ExpressionTsSecond*) override {}
⬜ 520        void visit(const ExpressionTsIncrement*) override {}
⬜ 521    };
⬜ 522    }  // namespace mongo
```

<br/>

3\. Update `📄 src/mongo/db/query/sbe_stage_builder_expression.cpp`, for example:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/query/sbe_stage_builder_expression.cpp
```c++
⬜ 471        void visit(const ExpressionDateSubtract* expr) final {}
⬜ 472        void visit(const ExpressionGetField* expr) final {}
⬜ 473        void visit(const ExpressionSetField* expr) final {}
🟩 474        void visit(const ExpressionTsSecond* expr) final {}
⬜ 475        void visit(const ExpressionTsIncrement* expr) final {}
⬜ 476    
⬜ 477    private:
```

<br/>

Also in the same file:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/query/sbe_stage_builder_expression.cpp
```c++
⬜ 702        void visit(const ExpressionDateSubtract* expr) final {}
⬜ 703        void visit(const ExpressionGetField* expr) final {}
⬜ 704        void visit(const ExpressionSetField* expr) final {}
🟩 705        void visit(const ExpressionTsSecond* expr) final {}
⬜ 706        void visit(const ExpressionTsIncrement* expr) final {}
⬜ 707    
⬜ 708    private:
```

<br/>

Still in the same file:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/query/sbe_stage_builder_expression.cpp
```c++
⬜ 2949           unsupportedExpression("$setField");
⬜ 2950       }
⬜ 2951   
🟩 2952       void visit(const ExpressionTsSecond* expr) final {
⬜ 2953           _context->ensureArity(1);
⬜ 2954   
⬜ 2955           auto tsSecondExpr = makeLocalBind(
```

<br/>

3\. Update `📄 src/mongo/db/cst/cst_pipeline_translation.cpp`, for instance:
<!-- NOTE-swimm-snippet: the lines below link your snippet to Swimm -->
### 📄 src/mongo/db/cst/cst_pipeline_translation.cpp
```c++
⬜ 639            case KeyFieldname::first:
⬜ 640                return make_intrusive<ExpressionFirst>(expCtx, std::move(expressions));
⬜ 641            case KeyFieldname::tsSecond:
🟩 642                return make_intrusive<ExpressionTsSecond>(expCtx, std::move(expressions));
⬜ 643            case KeyFieldname::tsIncrement:
⬜ 644                return make_intrusive<ExpressionTsIncrement>(expCtx, std::move(expressions));
⬜ 645            default:
```

<br/>

<!-- THIS IS AN AUTOGENERATED SECTION. DO NOT EDIT THIS SECTION DIRECTLY -->
### Swimm Note

<span id="f-hFSg">acceptVisitor</span>[^](#hFSg) - "src/mongo/db/pipeline/expression.h" L4185
```c
    void acceptVisitor(ExpressionConstVisitor* visitor) const final {
```

<span id="f-24yrKg">DateExpressionAcceptingTimeZone</span>[^](#24yrKg) - "src/mongo/db/pipeline/expression.h" L747
```c
class DateExpressionAcceptingTimeZone : public Expression {
```

<span id="f-Z1rHSsm">evaluate</span>[^](#Z1rHSsm) - "src/mongo/db/pipeline/expression.cpp" L7844
```c++
Value ExpressionTsSecond::evaluate(const Document& root, Variables* variables) const {
```

<span id="f-1YUCQJ">Expression</span>[^](#1YUCQJ) - "src/mongo/db/pipeline/expression.h" L161
```c
class Expression : public RefCountable {
```

<span id="f-1zjaOt">ExpressionCompare</span>[^](#1zjaOt) - "src/mongo/db/pipeline/expression.h" L1220
```c
class ExpressionCompare final : public ExpressionFixedArity<ExpressionCompare, 2> {
```

<span id="f-ZDExwG">ExpressionConstant</span>[^](#ZDExwG) - "src/mongo/db/pipeline/expression.h" L677
```c
class ExpressionConstant final : public Expression {
```

<span id="f-ZrN6LY">ExpressionDayOfMonth</span>[^](#ZrN6LY) - "src/mongo/db/pipeline/expression.h" L1542
```c
class ExpressionDayOfMonth final : public DateExpressionAcceptingTimeZone<ExpressionDayOfMonth> {
```

<span id="f-Z2M4eH">ExpressionFixedArity</span>[^](#Z2M4eH) - "src/mongo/db/pipeline/expression.h" L468
```c
class ExpressionFixedArity : public ExpressionNaryBase<SubClass> {
```

<span id="f-Zvf9D6">ExpressionFromAccumulator</span>[^](#Zvf9D6) - "src/mongo/db/pipeline/expression.h" L488
```c
class ExpressionFromAccumulator
```

<span id="f-KogNe">ExpressionMeta</span>[^](#KogNe) - "src/mongo/db/pipeline/expression.h" L2255
```c
class ExpressionMeta final : public Expression {
```

<span id="f-Z6B4Yv">ExpressionObject</span>[^](#Z6B4Yv) - "src/mongo/db/pipeline/expression.h" L2449
```c
class ExpressionObject final : public Expression {
```

<span id="f-rz60e">ExpressionSubtract</span>[^](#rz60e) - "src/mongo/db/pipeline/expression.h" L3166
```c
class ExpressionSubtract final : public ExpressionFixedArity<ExpressionSubtract, 2> {
```

<span id="f-PRwTQ">ExpressionTsSecond</span>[^](#PRwTQ) - "src/mongo/db/pipeline/expression.h" L4165
```c
class ExpressionTsSecond final : public ExpressionFixedArity<ExpressionTsSecond, 1> {
```

<span id="f-ZmhFFx">ExpressionType</span>[^](#ZmhFFx) - "src/mongo/db/pipeline/expression.h" L3380
```c
class ExpressionType final : public ExpressionFixedArity<ExpressionType, 1> {
```

<span id="f-13deN9">ExpressionVariadic</span>[^](#13deN9) - "src/mongo/db/pipeline/expression.h" L437
```c
class ExpressionVariadic : public ExpressionNaryBase<SubClass> {
```

<span id="f-15MGit">getOpName</span>[^](#15MGit) - "src/mongo/db/pipeline/expression.h" L4177
```c
    const char* getOpName() const final {
```

<br/>

This file was generated by Swimm. [Click here to view it in the app](https://swimm-web-app.web.app/repos/Z2l0aHViJTNBJTNBbW9uZ28lM0ElM0FzYWFyLXN3aW1t/docs/fe1qx).