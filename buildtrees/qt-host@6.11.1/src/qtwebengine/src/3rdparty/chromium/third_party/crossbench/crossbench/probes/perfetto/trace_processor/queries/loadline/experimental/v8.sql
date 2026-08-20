INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS metric;

CREATE VIEW metric
AS
SELECT
  interval_id,
  'v8_optimize' AS metric_name,
  'cpu_sec' AS unit,
  SUM(thread_span_dur) / 1e9 AS value
FROM interesting_slice_span
WHERE
  name IN (
    'V8.OptimizeBackground',
    'V8.OptimizeCode',
    'V8.InstallOptimizedFunctions',
    'V8.FinalizeMaglevConcurrentCompilation')
GROUP BY interval_id
UNION ALL
SELECT
  interval_id,
  'v8_parse' AS metric_name,
  'cpu_sec' AS unit,
  sum(coalesce(thread_span_dur, 0)) / 1e9 AS value
FROM interesting_slice_span
WHERE
  name IN (
    'V8.ParseFunction',
    'V8.ParseProgram',
    'v8.parseOnBackground')  -- we're double counting batch baseline compoilation here
GROUP BY interval_id
UNION ALL
SELECT
  interval_id,
  'v8_compile' AS metric_name,
  'cpu_sec' AS unit,
  sum(coalesce(thread_span_dur, 0)) / 1e9 AS value
FROM interesting_slice_span
WHERE
  name IN (
    'V8.CompileIgnition',
    'V8.CompileIgnitionFinalization',
    'V8.FinalizeBaselineConcurrentCompilation')
  OR (  -- hack until we have a proper baseline compilation slice
    name = 'ThreadPool_RunTask'
    AND EXTRACT_ARG(arg_set_id, 'task.posted_from.function_name') = "ConcurrentBaselineCompiler")
GROUP BY interval_id
UNION ALL
SELECT
  interval_id,
  'style_and_layout' AS metric_name,
  'cpu_sec' AS unit,
  SUM(thread_span_dur) / 1e9 AS value
FROM interesting_slice_span
WHERE name = 'Blink.ForcedStyleAndLayout.UpdateTime' OR name = 'LocalFrameView::RunStyleAndLayoutLifecyclePhases'
GROUP BY interval_id
UNION ALL
SELECT
  interval_id,
  'blink_parse_style' AS name,
  'cpu_sec' AS unit,
  SUM(thread_span_dur) / 1e9 AS parse_style
FROM interesting_slice_span
WHERE name = 'Blink.ParseStyleSheet.UpdateTime'
GROUP BY interval_id
UNION ALL
SELECT
  interval_id,
  'gpu_command_flush' AS metric_name,
  'cpu_sec' AS unit,
  SUM(thread_span_dur) / 1e9 AS value
FROM interesting_slice_span
WHERE name = 'CommandBufferStub::OnAsyncFlush'
GROUP BY interval_id;

SELECT * FROM metric;
