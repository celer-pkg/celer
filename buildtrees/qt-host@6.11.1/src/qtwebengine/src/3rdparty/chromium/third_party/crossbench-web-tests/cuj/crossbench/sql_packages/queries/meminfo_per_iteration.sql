-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
INCLUDE PERFETTO MODULE sql_packages.queries.meminfo_total;

-- Compute the slope of the least-squares approximation line for meminfo samples
-- collected with the same title across multiple iterations.
-- x-axis: it_id; slope is change in memory per iteration
-- y-axis: pss_total_mb + swap_total_mb, combine pss and swap to track memory

drop view if exists per_iteration;

-- First, average multiple meminfo samples in the same iteration with the same
-- title together so that there is only one y value per iteration.
create view per_iteration as
select
  title,
  CAST(it_id as REAL) as x,
  AVG(pss_total_mb + swap_total_mb) as y
from meminfo_total_output
-- Filter out setup iterations, keeping only integer it_ids.
where it_id = CAST(CAST(it_id as INTEGER) as TEXT)
group by x, title;

drop view if exists mean;

create view mean as
select
  title,
  AVG(x) as x,
  AVG(y) as y
from per_iteration
group by title;

DROP TABLE IF EXISTS meminfo_per_iteration_output;

-- Compute the least squares slope value using the formula from:
--  https://en.wikipedia.org/wiki/Simple_linear_regression
-- slope = sum((x_i - x_mean) * (y_i - y_mean)) / sum((x_i - x_mean)^2)
-- We do this by computing the mean above and joining it to each sample.
CREATE PERFETTO TABLE meminfo_per_iteration_output AS
SELECT
  per_iteration.title as title,
  SUM((per_iteration.x - mean.x) * (per_iteration.y - mean.y)) / SUM((per_iteration.x - mean.x) * (per_iteration.x - mean.x)) as per_iteration_mb
FROM per_iteration join mean on per_iteration.title = mean.title
GROUP BY per_iteration.title;
