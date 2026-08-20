INCLUDE PERFETTO MODULE ext.first_presentation_time;

CREATE OR REPLACE PERFETTO FUNCTION loadline2_cnn_article_score()
RETURNS FLOAT
AS
SELECT
  -- Multiply by 60 to make the score per minutes rather than per second.
  60e9 / (
    get_presentation_time('LoadLine2/cnn_article/menu_shown')
    - get_event_time('LoadLine2/*/cnn_article_start'));

