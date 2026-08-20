INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS sequence_manager_metric;

CREATE VIEW sequence_manager_metric
AS
SELECT interval_id, COUNT(*) AS value, 'tasks_started' AS metric_name, 'count' AS unit
FROM interesting_slice_start
WHERE name = 'ThreadControllerImpl::RunTask'
GROUP BY interval_id
UNION ALL
SELECT interval_id, COUNT(*) AS value, 'tasks_posted' AS metric_name, 'count' AS unit
FROM interesting_slice_start
WHERE name = 'SequenceManager PostTask'
GROUP BY interval_id
UNION ALL
SELECT interval_id, COUNT(*) AS value, 'tasks_completed' AS metric_name, 'count' AS unit
FROM interesting_slice_end
WHERE name = 'ThreadControllerImpl::RunTask'
GROUP BY interval_id;

SELECT * FROM sequence_manager_metric;
