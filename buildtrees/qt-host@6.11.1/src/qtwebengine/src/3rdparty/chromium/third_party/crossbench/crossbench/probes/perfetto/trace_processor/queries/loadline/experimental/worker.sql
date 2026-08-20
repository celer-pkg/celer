INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS worker_metric;

CREATE VIEW worker_metric
AS
WITH
  data AS (
    SELECT interval_id, thread.name AS thread_name
    FROM interesting_slice_start, thread_track
    ON track_id = thread_track.id,
    thread
    USING (utid)
    WHERE interesting_slice_start.name = 'WorkerThread::InitializeWorkerContext'
  )
SELECT
  interval_id,
  'web_worker_count' AS metric_name,
  'count' AS unit,
  COUNT(*) AS value
FROM data
WHERE thread_name LIKE 'DedicatedWorker%'
UNION ALL
SELECT
  interval_id,
  'service_worker_count' AS metric_name,
  'count' AS unit,
  COUNT(*) AS value
FROM data
WHERE thread_name LIKE 'ServiceWorker%';

SELECT * FROM worker_metric;
