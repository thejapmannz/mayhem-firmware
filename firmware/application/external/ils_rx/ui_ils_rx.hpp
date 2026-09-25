/*
 * Copyright (C) 2026 PortaPack Mayhem
 * Copyright (C) 2026 thejapmannz
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __UI_ILS_RX_H__
#define __UI_ILS_RX_H__

#include "audio.hpp"
#include "baseband_api.hpp"
#include "file_path.hpp"
#include "log_file.hpp"
#include "rtc_time.hpp"
#include "portapack.hpp"
#include "ui.hpp"
#include "ui_audio.hpp"
#include "ui_channel.hpp"
#include "ui_freq_field.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "ui_rssi.hpp"

namespace ui::external_app::ils_rx {

/*class IlsLogger {
   public:
    Optional<File::Error> append(const std::filesystem::path& filename) {
        return log_file.append(filename);
    }

    void write_header();
    void log_status(const IlsRxStatusDataMessage& message);

   private:
    LogFile log_file{};
};*/
/*
class IlsCdiIndicator : public Widget {
   public:
    IlsCdiIndicator(Point position);

    void set_ddm(uint16_t ddm_percent);
    void set_valid(bool valid);
    void set_loc_glidepath(bool is_glidepath);

    void paint(Painter& painter) override;

   private:

    uint16_t ddm_percent_{0};
    bool valid_{false};
    bool is_glidepath_{false};
};*/

class IlsRxView : public View {
   public:
    IlsRxView(NavigationView& nav);
    ~IlsRxView();

    void focus() override;

    std::string title() const override { return "ILS RX"; }

   private:
    void start_receiver();
    void stop_receiver();
    void update_status();
    void on_ils_status(const IlsRxStatusDataMessage& message);
    void update_cdi();
    void refresh_radial();
    uint16_t calibrated_radial(uint16_t radial_deg) const;
    uint16_t smooth_radial(uint16_t radial_deg);
    void update_logging();

    NavigationView& nav_;
    bool running_{false};
    bool logging_{false};
    bool have_status_{false};
    uint16_t last_ddm_{0};
    bool last_valid_{false};
    uint8_t flag_state_{0};  // TO/FROM hysteresis: 0=unknown, 1=FROM, 2=TO
    // Circular exponential moving average of the radial. Each 100 ms estimate
    // is noisy (~10 deg std even when locked), so blend it along the shortest
    // arc in 1/64 deg fixed point to average correctly across the 0/360 deg
    // wrap without trig.
    bool radial_filter_valid_{false};
    int32_t radial_smoothed_fp_{0};

    RxFrequencyField field_frequency{
        {UI_POS_X(0), UI_POS_Y(0)},
        nav_};
    RFAmpField field_rf_amp{
        {UI_POS_X(13), UI_POS_Y(0)}};
    LNAGainField field_lna{
        {UI_POS_X(15), UI_POS_Y(0)}};
    VGAGainField field_vga{
        {UI_POS_X(18), UI_POS_Y(0)}};
    AudioVolumeField field_volume{
        {screen_width - 2 * 8, UI_POS_Y(0)}};

    RSSI rssi{
        {UI_POS_X(21), 0, UI_POS_WIDTH_REMAINING(21) - UI_POS_WIDTH(2), 4}};
    Channel channel{
        {UI_POS_X(21), 5, UI_POS_WIDTH_REMAINING(21) - UI_POS_WIDTH(2), 4}};
    Audio audio{
        {UI_POS_X(21), 10, UI_POS_WIDTH_REMAINING(21) - UI_POS_WIDTH(2), 4}};

    NumberField field_calibration{
        {UI_POS_X(14), UI_POS_Y(7)},
        4,
        {-359, 359},
        1,
        ' '};

    Labels labels{
        {{UI_POS_X(0), UI_POS_Y(1)}, "Status:", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), UI_POS_Y(3)}, "Decoder:", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), UI_POS_Y(5)}, "Rec. Radial:", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), UI_POS_Y(7)}, "Calibration:", Theme::getInstance()->fg_light->foreground}};

    Text text_status{
        {UI_POS_X(14), UI_POS_Y(1), UI_POS_WIDTH_REMAINING(14), UI_POS_HEIGHT(1)},
        "Idle"};
    Text text_next{
        {UI_POS_X(14), UI_POS_Y(3), UI_POS_WIDTH_REMAINING(14), UI_POS_HEIGHT(1)},
        "Pending"};
    Text text_radial{
        {UI_POS_X(14), UI_POS_Y(5), UI_POS_WIDTH_REMAINING(14), UI_POS_HEIGHT(1)},
        "--"};
    Text text_calib_unit{
        {UI_POS_X(19), UI_POS_Y(7), UI_POS_WIDTH_REMAINING(19), UI_POS_HEIGHT(1)},
        "deg"};

    Text text_cdi_title{
        {UI_POS_X(0), UI_POS_Y(9), UI_POS_WIDTH_REMAINING(0), UI_POS_HEIGHT(1)},
        "Course Deviation Indicator"};

    // IlsCdiIndicator cdi_indicator{
        // {UI_POS_X(0), UI_POS_Y(10)}};

    std::unique_ptr<IlsLogger> logger{};

    Button button_start_stop{
        {UI_POS_X(9), UI_POS_Y(14), UI_POS_WIDTH(12), UI_POS_HEIGHT(2)},
        "Start"};

    Checkbox check_log{
        {UI_POS_X(7), UI_POS_Y(17)},
        3,
        "LOG to SD Card",
        false};

    MessageHandlerRegistration message_handler_ils_status{
        Message::ID::IlsRxStatusData,
        [this](const Message* p) {
            const auto message = *reinterpret_cast<const IlsRxStatusDataMessage*>(p);
            on_ils_status(message);
        }};
};

}  // namespace ui::external_app::ils_rx

#endif  // __UI_ILS_RX_H__
