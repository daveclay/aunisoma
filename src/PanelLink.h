#ifndef C_AUNISOMA_PANEL_LINK_H
#define C_AUNISOMA_PANEL_LINK_H

class PanelLink {
public:
    static constexpr int PANEL_COUNT = 20;
    static constexpr int COLOR_HEX_LEN = 6;

    void begin();
    bool enumerate();
    bool map_panels(const char* panel_ids);
    void map_panels_until_ok(const char* panel_ids);

    bool send_colors(const char* color_hex, char pir_out[PANEL_COUNT]);
};

#endif
