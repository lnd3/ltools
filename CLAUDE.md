# ltools project

## What This Is
`ltools` is a modern c++ library consisting of a collection of packages and tools for modern c++ development, organized around the build system `bs` from `ldeps`. Packages found at `packages/**`.

## Notable Packages
- **Node graph system**: Package `ecs`. Location `packages/ecs`. `NodeGraphOp` base class, inputs/outputs, cache blocks.
- **ECS**: Package `ecs`. Location `packages/nodegraph`. Entity component system. Entity contains components. Components contain data structures. Iterate over component groups in systems and do work on components and/or entities. Access any data by name or id.

## Dependencies
- **ldeps**: Location `deps/ldeps`. See `deps/ldeps/CLAUDE.md`.

## Coding Patterns
As required by `bs`, packages have a certain layout: `${package_name}/[include/${package_name}/|source/common|tests/common|]`.

### Node graph
- Node inputs: `AddInput2("Name")` for connected, `AddInput("Name", default, size, min, max)` for params
- Node outputs: `AddOutput("name")`
- Process signature: `Process(int32_t numSamples, int32_t numCacheSamples, inputs, outputs)`
- Registration: `schema.RegisterNodeType("Category", ID, "Name", "Description")` + switch case

