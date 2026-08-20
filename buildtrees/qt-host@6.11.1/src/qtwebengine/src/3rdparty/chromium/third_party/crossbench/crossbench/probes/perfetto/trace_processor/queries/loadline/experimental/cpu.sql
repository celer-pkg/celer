INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS sched_slice_type_in;

CREATE VIEW sched_slice_type_in
AS
SELECT
  sched_slice.*,
  CASE
    WHEN thread.name = 'CrRendererMain' THEN 'CrRendererMain'
    WHEN thread.name GLOB 'ThreadPool*' THEN 'ThreadPool'
    WHEN thread.name GLOB '*IOThread' THEN 'IOThread'
    WHEN thread.name = 'NetworkService' THEN 'NetworkService'
    WHEN thread.name = 'Compositor' THEN 'Compositor'
    WHEN thread.name = 'VizCompositorThread' THEN 'VizCompositorThread'
    WHEN EXTRACT_ARG(arg_set_id, 'chrome.process_type') IS NOT NULL
      THEN 'Other_' || EXTRACT_ARG(arg_set_id, 'chrome.process_type')
    WHEN thread.name GLOB 'binder*' THEN 'binder'
    WHEN thread.name GLOB 'surfaceflinger*' THEN 'surfaceflinger'
    WHEN thread.name = 'HeapTaskDaemon' THEN 'HeapTaskDaemon'
    ELSE 'Other'
    END AS thread_type
FROM sched_slice
LEFT JOIN thread
  USING (utid)
LEFT JOIN process
  USING (upid)
WHERE
  utid <> 0;

DROP TABLE IF EXISTS sched_slice_type_span;

CREATE VIRTUAL TABLE sched_slice_type_span
USING
  SPAN_JOIN(interesting_interval, sched_slice_type_in PARTITIONED cpu);

DROP VIEW IF EXISTS cpu_metric;

CREATE VIEW cpu_metric
AS
SELECT
  thread_type,
  interval_id,
  "thread_time" AS metric_name,
  "cpu_sec" AS unit,
  sum(dur) / 1e9 AS value
FROM sched_slice_type_span
GROUP BY thread_type, interval_id;

SELECT * FROM cpu_metric;
