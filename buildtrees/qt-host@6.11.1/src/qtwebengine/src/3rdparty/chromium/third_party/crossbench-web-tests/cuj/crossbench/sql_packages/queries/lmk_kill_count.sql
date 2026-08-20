-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.web_tests_common.iterations;

drop view if exists lmk_kill_ts;
create view
  lmk_kill_ts as
select
  ts
from
  slice
where
  name = 'lmk_kill_occurred';

DROP TABLE IF EXISTS lmk_kill_count_output;
CREATE PERFETTO TABLE lmk_kill_count_output AS
select
  iterations.id as it_id,
  count(lmk_kill_ts.ts) as kill_count
from
  iterations
  left join lmk_kill_ts on lmk_kill_ts.ts >= iterations.start
  and lmk_kill_ts.ts <= iterations.end;
