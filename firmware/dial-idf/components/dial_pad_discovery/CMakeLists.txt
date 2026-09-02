idf_component_register(
    SRCS "dial_pad_discovery.c"
    INCLUDE_DIRS "."
    REQUIRES dial_state
    PRIV_REQUIRES esp_http_client cjson dial_net esp_timer)
