INCLUDE PERFETTO MODULE ext.loadline_amazon_product;
INCLUDE PERFETTO MODULE ext.loadline_cnn_article;
INCLUDE PERFETTO MODULE ext.loadline_globo_homepage;
INCLUDE PERFETTO MODULE ext.loadline_google_doc;
INCLUDE PERFETTO MODULE ext.loadline_google_search_result;
INCLUDE PERFETTO MODULE ext.loadline_wikipedia_article;
INCLUDE PERFETTO MODULE ext.loadline_youtube_video;

CREATE OR REPLACE PERFETTO FUNCTION loadline_get_name()
RETURNS STRING
AS
SELECT DISTINCT substr(name, length('LoadLine/') + 1)
FROM slice
WHERE category = 'blink.user_timing' AND name GLOB 'LoadLine/*'
LIMIT 1;

CREATE OR REPLACE PERFETTO FUNCTION loadline_benchmark_score()
RETURNS FLOAT
AS
SELECT
  CASE loadline_get_name()
    WHEN 'loadline-phone/amazon_product' THEN loadline_amazon_product_score()
    WHEN 'loadline-tablet/amazon_product' THEN loadline_amazon_product_score()
    WHEN 'loadline-phone/cnn_article' THEN loadline_phone_cnn_article_score()
    WHEN 'loadline-tablet/cnn_article' THEN loadline_tablet_cnn_article_score()
    WHEN 'loadline-phone/globo_homepage' THEN loadline_globo_homepage_score()
    WHEN 'loadline-tablet/google_doc' THEN loadline_google_doc_score()
    WHEN 'loadline-phone/google_search_result' THEN loadline_google_search_result_score()
    WHEN 'loadline-tablet/google_search_result' THEN loadline_google_search_result_score()
    WHEN 'loadline-phone/wikipedia_article' THEN loadline_wikipedia_article_score()
    WHEN 'loadline-tablet/youtube_video' THEN loadline_youtube_video_score()
    ELSE NULL
    END AS score;
