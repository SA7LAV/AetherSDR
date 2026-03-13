#pragma once

#include <QWidget>

class QPushButton;
class QSlider;
class QLabel;

namespace AetherSDR {

class SliceModel;

// RX Applet — controls for a single receive slice.
//
// Shows and controls:
//  • RX antenna selector (ANT1/ANT2)
//  • Filter width presets (1.8/2.1/2.4/2.7/3.3/6.0 kHz)
//  • AGC mode (OFF/SLOW/MED/FAST)
//  • Squelch on/off + level slider
//  • DSP toggles: NB, NR, ANF
//  • RIT on/off + Hz offset ±
//  • XIT on/off + Hz offset ±
class RxApplet : public QWidget {
    Q_OBJECT

public:
    explicit RxApplet(QWidget* parent = nullptr);

    // Attach to a slice; pass nullptr to detach.
    void setSlice(SliceModel* slice);

private:
    void buildUI();
    void connectSlice(SliceModel* s);
    void disconnectSlice(SliceModel* s);

    void applyFilterPreset(int widthHz);
    void updateFilterButtons();
    void updateAgcButtons();

    SliceModel* m_slice{nullptr};

    // ANT
    QPushButton* m_antBtns[2]{};   // ANT1, ANT2

    // Filter presets (widths in Hz)
    static constexpr int FILTER_WIDTHS[6] = {1800, 2100, 2400, 2700, 3300, 6000};
    QPushButton* m_filterBtns[6]{};

    // AGC mode buttons (off / slow / med / fast)
    static constexpr const char* AGC_MODES[4] = {"off", "slow", "med", "fast"};
    QPushButton* m_agcBtns[4]{};

    // Squelch
    QPushButton* m_sqlBtn{nullptr};
    QSlider*     m_sqlSlider{nullptr};
    QLabel*      m_sqlLabel{nullptr};

    // DSP
    QPushButton* m_nbBtn{nullptr};
    QPushButton* m_nrBtn{nullptr};
    QPushButton* m_anfBtn{nullptr};

    // RIT
    QPushButton* m_ritOnBtn{nullptr};
    QLabel*      m_ritLabel{nullptr};
    QPushButton* m_ritMinus{nullptr};
    QPushButton* m_ritPlus{nullptr};
    QPushButton* m_ritClear{nullptr};

    // XIT
    QPushButton* m_xitOnBtn{nullptr};
    QLabel*      m_xitLabel{nullptr};
    QPushButton* m_xitMinus{nullptr};
    QPushButton* m_xitPlus{nullptr};
    QPushButton* m_xitClear{nullptr};

    static constexpr int RIT_STEP_HZ = 10;
};

} // namespace AetherSDR
