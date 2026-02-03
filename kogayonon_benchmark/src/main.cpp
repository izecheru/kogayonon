#include "benchmark.hpp"
/**
 * @brief Performance benchmarks for core Kogayonon systems.
 *
 * This benchmark suite measures the performance of entity creation and
 * JSON serialization/deserialization at various scales.
 *
 * @date 03-02-2026
 *
 * @par Test Environment
 * - CPU: 12 cores @ 3.6 GHz
 * - Build: Debug (timings may be affected)
 * - Caches:
 *   - L1 Data: 32 KiB (x6)
 *   - L1 Instruction: 32 KiB (x6)
 *   - L2 Unified: 512 KiB (x6)
 *   - L3 Unified: 16 MiB (x1)
 *
 * @par Benchmarks
 *
 * @par Entity Creation
 * | Entities | Time (CPU) |
 * |----------|------------|
 * | 1,000    | ~0.000 s   |
 * | 100,000  | ~0.040 s   |
 * | 1,000,000| ~0.391 s   |
 *
 * @par JSON Serialization
 * | Entities | Time (CPU) |
 * |----------|------------|
 * | 1,000    | ~0.030 s   |
 * | 100,000  | ~2.86 s    |
 * | 1,000,000| ~29.4 s    |
 *
 * @par JSON Deserialization
 * | Entities | Time (CPU) |
 * |----------|------------|
 * | 1,000    | ~46.6 s    |
 * | 100,000  | ~44.4 s    |
 * | 1,000,000| ~35.8 s    |
 *
 * @note JSON deserialization dominates runtime and shows non-linear scaling.
 * @note Results were collected from a DEBUG build; RELEASE builds are expected
 *       to perform significantly better.
 */

/**
 * @brief Performance benchmarks for core Kogayonon systems (Release build).
 *
 * This benchmark suite evaluates entity creation and JSON
 * serialization/deserialization performance using compiler optimizations
 * enabled.
 *
 * @date 2026-02-03
 *
 * @par Test Environment
 * - CPU: 12 cores @ 3.6 GHz
 * - Build: Release
 * - Caches:
 *   - L1 Data: 32 KiB (x6)
 *   - L1 Instruction: 32 KiB (x6)
 *   - L2 Unified: 512 KiB (x6)
 *   - L3 Unified: 16 MiB (x1)
 *
 * @par Benchmarks
 *
 * @par Entity Creation
 * | Entities | Time (CPU) |
 * |----------|------------|
 * | 1,000    | ~0.000 s   |
 * | 100,000  | ~0.002 s   |
 * | 1,000,000| ~0.013 s   |
 *
 * @par JSON Serialization
 * | Entities | Time (CPU) |
 * |----------|------------|
 * | 1,000    | ~0.002 s   |
 * | 100,000  | ~0.176 s   |
 * | 1,000,000| ~1.92 s    |
 *
 * @par JSON Deserialization
 * | Entities | Time (CPU) |
 * |----------|------------|
 * | 1,000    | ~4.77 s    |
 * | 100,000  | ~4.70 s    |
 * | 1,000,000| ~4.67 s    |
 *
 * @note Release build shows orders-of-magnitude improvement over Debug,
 *       particularly for entity creation and JSON serialization.
 * @note JSON deserialization performance is largely size-invariant,
 *       suggesting a fixed-cost or I/O-dominated workload.
 */

BENCHMARK( kogayonon_benchmark::BM_CreateEntities )
  ->Arg( 1000 )
  ->Arg( 100000 )
  ->Arg( 1000000 )
  ->Unit( benchmark::kSecond );

BENCHMARK( kogayonon_benchmark::BM_JsonSerialization )
  ->Arg( 1000 )
  ->Arg( 100000 )
  ->Arg( 1000000 )
  ->Unit( benchmark::kSecond );

// #warning "This must be called AFTER serialization"
BENCHMARK( kogayonon_benchmark::BM_JsonDeserialization )
  ->Arg( 1000 )
  ->Arg( 100000 )
  ->Arg( 1000000 )
  ->Unit( benchmark::kSecond );

// this is very slow, for 100k transforms we would get 40seconds and for a million 436seconds, roughly 7 minutes
// compared to 34s on json
// JSON IS 10 TIMES FASTER

// BENCHMARK( kogayonon_benchmark::BM_Serialization )
//   ->Arg( 1000 )
//   ->Arg( 100000 )
//   ->Arg( 1000000 )
//   ->Unit( benchmark::kSecond );

BENCHMARK_MAIN();