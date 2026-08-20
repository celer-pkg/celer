-- Create tables with page loading breakdown into stages for the LoadLine
-- benchmark.
-- TODO(crbug.com/425325733): Support LoadLine 2 as well.

INCLUDE PERFETTO MODULE ext.loadline_benchmark;

DROP VIEW IF EXISTS loadline_presentation;
CREATE VIEW loadline_presentation AS
SELECT
  first_navigation_start() + 60e9 / loadline_benchmark_score() AS presentation;

-- Finds the "Commit sent" moment which is the time when the browser gets the
-- response from the network stack.
DROP VIEW IF EXISTS loadline_request;
CREATE VIEW loadline_request AS
SELECT MIN(ts) AS end_request
FROM slice
WHERE
  name = 'CommitSentToFirstSubresourceLoadStart'
  AND ts >= first_navigation_start();

DROP VIEW IF EXISTS loadline_renderer_ready;
CREATE VIEW loadline_renderer_ready AS
SELECT MIN(ts) AS renderer_ready
FROM slice
WHERE
  name = 'DocumentLoader::CommitNavigation'
  AND ts >= first_navigation_start();

-- Find the frame in the pipeline which was chosen as the "loading complete"
-- moment for the purpose of LoadLine score. The exact end timestamp might
-- differ a little due to rounding error, so we allow 1ms discrepancy while
-- matching. This should not match any extra frames since frames are aligned to
-- vsyncs, and vsync interval is usually 8-17ms.
DROP VIEW IF EXISTS loadline_frame;
CREATE VIEW loadline_frame AS
SELECT id
FROM slice, loadline_presentation
WHERE
  name = 'PipelineReporter'
  AND ts + dur BETWEEN presentation - 1e6 AND presentation + 1e6
  AND COALESCE(
    extract_arg(arg_set_id, 'frame_reporter.state'),
    -- TODO(crbug.com/409484302): Remove once Chrome migrates from
    -- ChromeTrackEvent.chrome_frame_reporter to
    -- ChromeTrackEvent.frame_reporter.
    extract_arg(arg_set_id, 'chrome_frame_reporter.state')
  ) = 'STATE_PRESENTED_ALL'
ORDER BY ts
LIMIT 1;

DROP VIEW IF EXISTS loadline_frame_commit;
CREATE VIEW loadline_frame_commit AS
SELECT child.ts + child.dur AS frame_commit
FROM loadline_frame, descendant_slice(loadline_frame.id) AS child
WHERE child.name = 'Commit';

DROP VIEW IF EXISTS loadline_submit_compositor_frame;
CREATE VIEW loadline_submit_compositor_frame AS
SELECT child.ts AS submit_compositor_frame
FROM loadline_frame, descendant_slice(loadline_frame.id) AS child
WHERE child.name = 'SubmitCompositorFrameToPresentationCompositorFrame';

DROP VIEW IF EXISTS loadline_frame_swap;
CREATE VIEW loadline_frame_swap AS
SELECT child.ts + child.dur AS frame_swap
FROM loadline_frame, descendant_slice(loadline_frame.id) AS child
WHERE child.name = 'StartDrawToSwapStart';

DROP VIEW IF EXISTS loadline_stages;
CREATE VIEW loadline_stages AS
SELECT
  first_navigation_start() AS navigation_start,
  end_request,
  renderer_ready,
  frame_commit,
  submit_compositor_frame,
  frame_swap,
  presentation
FROM loadline_presentation, loadline_request, loadline_renderer_ready,
     loadline_frame_commit, loadline_submit_compositor_frame, loadline_frame_swap;

