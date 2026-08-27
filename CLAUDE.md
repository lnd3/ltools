# ltools project

## What This Is
`ltools` is a modern C++ library consisting of a collection of packages and tools for modern C++ development, organized around the build system `bs` from `ldeps`. Packages found at `packages/**`.

## Notable Packages
- **nodegraph**: Package `nodegraph`. Location `packages/nodegraph`. Node graph system. `NodeGraphOp` base class, `NodeGraphOpCached` for cached operations, inputs/outputs, cache blocks, `NodeGraphSchema` with Load/Save/JSON serialization and node factory. Includes trading-specific operations (data IO, detectors, filters, indicators). Namespace: `l::nodegraph`.
- **ecs**: Package `ecs`. Location `packages/ecs`. Entity component system. Entity contains components. Components contain data structures. Iterate over component groups in systems and do work on components and/or entities. Access any data by name or id. Namespace: `l::ecs`.
- **rendering**: Package `rendering`. Location `packages/rendering`. UI and rendering via ImGui/ImPlot. Namespace: `l::ui`.
- **serialization**: Package `serialization`. Location `packages/serialization`. JSON serialization framework (`JsonSerializationBase`, `JsonBuilder`, `JsonValue`). Used by `NodeGraphSchema` for persistence. Namespace: `l::serialization`.
- **math**: Package `math`. Location `packages/math`. Math utilities, tweening, algorithms. Namespace: `l::math`.
- **network**: Package `network`. Location `packages/network`. Network communication. Namespace: `l::network`.
- **nn**: Package `nn`. Location `packages/nn`. Neural network utilities. Namespace: `l::nn`.
- **memory**: Package `memory`. Location `packages/memory`. Containers and memory management. Namespace: `l::container`.
- **concurrency**: Package `concurrency`. Location `packages/concurrency`. Threading and synchronization. Namespace: `l::concurrency`.
- **audio**: Package `audio`. Location `packages/audio`. Audio utilities. Namespace: `l::audio`.
- **hid**: Package `hid`. Location `packages/hid`. Human input device and MIDI. Namespace: `l::hid`.
- **physics**: Package `physics`. Location `packages/physics`. Physics simulation and octree. Namespace: `l::physics`.
- **crypto**: Package `crypto`. Location `packages/crypto`. Cryptography utilities. Namespace: `l::crypto`.
- **filesystem**: Package `filesystem`. Location `packages/filesystem`. File system utilities. Namespace: `l::filesystem`.
- **storage**: Package `storage`. Location `packages/storage`. File caching and storage. Namespace: `l::filecache`.
- **logging**: Package `logging`. Location `packages/logging`. Logging system. Namespace: `l::logging`.
- **tools**: Package `tools`. Location `packages/tools`. Signal utilities and tools. Namespace: `l::signals`.
- **meta**: Package `meta`. Location `packages/meta`. Reflection and type info. Namespace: `l::meta`.
- **testing**: Package `testing`. Location `packages/testing`. Test utilities. Namespace: `l::testing`.

## Dependencies
- **ldeps**: Location `deps/ldeps`. See `deps/ldeps/CLAUDE.md`.

## Coding Patterns
As required by `bs`, packages have a certain layout: `${package_name}/[include/${package_name}/|source/common|tests/common|]`.

### Math utilities — prefer ltools over std
- **`l::math::max2(a, b)` / `l::math::min2(a, b)`** instead of `std::max` / `std::min`.
  These are the project-standard alternatives and avoid ambiguity issues with MSVC and
  Windows headers that define `max`/`min` macros. Always use these in code that touches
  ltools or TradeFlow; do not use `std::max` / `std::min`.
- **`l::math::clamp(v, lo, hi)`** instead of `std::clamp`.
- Header: `math/MathAlgorithm.h` (or included transitively via most ltools headers).

### Node graph
- Base class: `NodeGraphOp` in `packages/nodegraph/include/nodegraph/core/NodeGraphBase.h`
- Cached base class: `NodeGraphOpCached` — adds `ProcessWriteCached()` and `ProcessReadCached()` for buffered processing
- Schema: `NodeGraphSchema` inherits `JsonSerializationBase` and `NodeFactoryBase` — provides `Load()`, `Save()`, `RegisterNodeType()`, `NewNode()`
- Node inputs: `AddInput2("Name")` for connected, `AddInput("Name", default, size, min, max)` for params, `AddConstant("Name", default, size, min, max)` for constants
- Node outputs: `AddOutput("name")`
- Process signature: `Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&)`
- Registration: `schema.RegisterNodeType("Category", ID, "Name", "Description")` + switch case
- Node creation: `group.NewNode<OpType>(id, NodeType::Default)` — NodeType required (`Default`, `ExternalInput`, `ExternalOutput`)
