-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
DROP TABLE IF EXISTS cpu_usage_stat_output;

CREATE PERFETTO TABLE cpu_usage_stat_output
AS
WITH
  cpu_time_table AS (
    SELECT
      *
    FROM
      (
        SELECT
          cpu,
          name,
          value - LAG (value, 1, value + 1) OVER (
            PARTITION BY
              name,
              cpu
            ORDER BY
              ts
          ) AS cputime
        FROM
          counter AS c
          JOIN cpu_counter_track AS cct ON c.track_id = cct.id
        WHERE
          cct.name LIKE 'cpu.times.%'
        ORDER BY
          ts,
          cpu
      )
    WHERE
      cputime != -1
  )
SELECT
  name,
  CAST(MAX(cputime) AS INTEGER) AS max_cputime,
  CAST(AVG(cputime) AS INTEGER) AS mean_cputime,
  CAST(SUM(cputime) AS INTEGER) AS total_cputime,
  CAST(PERCENTILE (cputime, 90) AS INTEGER) AS p90_cputime,
  CAST(PERCENTILE (cputime, 50) AS INTEGER) AS p50_cputime
FROM
  cpu_time_table
GROUP BY
  name;
