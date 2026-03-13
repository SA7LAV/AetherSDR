#include "RxApplet.h"
#include "models/SliceModel.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace AetherSDR {

// ─── Helpers ──────────────────────────────────────────────────────────────────

static QFrame* hLine()
{
    auto* f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Plain);
    f->setStyleSheet("color: #203040;");
    return f;
}

static QPushButton* toggleBtn(const QString& text, QWidget* parent = nullptr)
{
    auto* b = new QPushButton(text, parent);
    b->setCheckable(true);
    b->setFlat(true);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setFixedHeight(24);
    return b;
}

// ─── Construction ─────────────────────────────────────────────────────────────

RxApplet::RxApplet(QWidget* parent) : QWidget(parent)
{
    buildUI();
}

void RxApplet::buildUI()
{
    setMinimumWidth(200);

    const QString activeStyle =
        "QPushButton:checked { background-color: #0070c0; color: #ffffff; border: 1px solid #0090e0; }";
    const QString dspActiveStyle =
        "QPushButton:checked { background-color: #007050; color: #ffffff; border: 1px solid #00a070; }";
    const QString ritActiveStyle =
        "QPushButton:checked { background-color: #705000; color: #ffffff; border: 1px solid #a07000; }";

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);

    // ── Antenna ───────────────────────────────────────────────────────────────
    {
        auto* row = new QHBoxLayout;
        row->addWidget(new QLabel("ANT:"));
        const char* labels[] = {"ANT1", "ANT2"};
        for (int i = 0; i < 2; ++i) {
            m_antBtns[i] = toggleBtn(labels[i]);
            m_antBtns[i]->setStyleSheet(activeStyle);
            connect(m_antBtns[i], &QPushButton::clicked, this, [this, i](bool) {
                if (m_slice) m_slice->setRxAntenna(i == 0 ? "ANT1" : "ANT2");
            });
            row->addWidget(m_antBtns[i]);
        }
        root->addLayout(row);
    }

    root->addWidget(hLine());

    // ── Filter presets ────────────────────────────────────────────────────────
    {
        root->addWidget(new QLabel("Filter:"));
        auto* grid = new QGridLayout;
        grid->setSpacing(3);
        const char* labels[] = {"1.8K","2.1K","2.4K","2.7K","3.3K","6.0K"};
        for (int i = 0; i < 6; ++i) {
            m_filterBtns[i] = toggleBtn(labels[i]);
            m_filterBtns[i]->setStyleSheet(activeStyle);
            const int w = FILTER_WIDTHS[i];
            connect(m_filterBtns[i], &QPushButton::clicked, this, [this, w](bool) {
                applyFilterPreset(w);
            });
            grid->addWidget(m_filterBtns[i], i / 3, i % 3);
        }
        root->addLayout(grid);
    }

    root->addWidget(hLine());

    // ── AGC mode ──────────────────────────────────────────────────────────────
    {
        auto* row = new QHBoxLayout;
        row->addWidget(new QLabel("AGC:"));
        const char* labels[] = {"Off","Slw","Med","Fst"};
        for (int i = 0; i < 4; ++i) {
            m_agcBtns[i] = toggleBtn(labels[i]);
            m_agcBtns[i]->setStyleSheet(activeStyle);
            const QString mode = AGC_MODES[i];
            connect(m_agcBtns[i], &QPushButton::clicked, this, [this, mode](bool) {
                if (m_slice) m_slice->setAgcMode(mode);
            });
            row->addWidget(m_agcBtns[i]);
        }
        root->addLayout(row);
    }

    root->addWidget(hLine());

    // ── Squelch ───────────────────────────────────────────────────────────────
    {
        auto* row = new QHBoxLayout;
        m_sqlBtn = toggleBtn("SQL");
        m_sqlBtn->setFixedWidth(40);
        m_sqlBtn->setStyleSheet(dspActiveStyle);
        row->addWidget(m_sqlBtn);

        m_sqlSlider = new QSlider(Qt::Horizontal);
        m_sqlSlider->setRange(0, 100);
        m_sqlSlider->setValue(20);
        row->addWidget(m_sqlSlider, 1);

        m_sqlLabel = new QLabel("20");
        m_sqlLabel->setFixedWidth(28);
        m_sqlLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        row->addWidget(m_sqlLabel);

        root->addLayout(row);

        connect(m_sqlBtn, &QPushButton::toggled, this, [this](bool on) {
            if (m_slice) m_slice->setSquelch(on, m_sqlSlider->value());
        });
        connect(m_sqlSlider, &QSlider::valueChanged, this, [this](int v) {
            m_sqlLabel->setText(QString::number(v));
            if (m_slice && m_sqlBtn->isChecked())
                m_slice->setSquelch(true, v);
        });
    }

    root->addWidget(hLine());

    // ── DSP toggles ───────────────────────────────────────────────────────────
    {
        auto* row = new QHBoxLayout;
        m_nbBtn  = toggleBtn("NB");
        m_nrBtn  = toggleBtn("NR");
        m_anfBtn = toggleBtn("ANF");
        for (auto* b : {m_nbBtn, m_nrBtn, m_anfBtn})
            b->setStyleSheet(dspActiveStyle);

        connect(m_nbBtn,  &QPushButton::toggled, this, [this](bool on) {
            if (m_slice) m_slice->setNb(on);
        });
        connect(m_nrBtn,  &QPushButton::toggled, this, [this](bool on) {
            if (m_slice) m_slice->setNr(on);
        });
        connect(m_anfBtn, &QPushButton::toggled, this, [this](bool on) {
            if (m_slice) m_slice->setAnf(on);
        });

        row->addWidget(m_nbBtn);
        row->addWidget(m_nrBtn);
        row->addWidget(m_anfBtn);
        root->addLayout(row);
    }

    root->addWidget(hLine());

    // ── RIT ───────────────────────────────────────────────────────────────────
    {
        auto* row = new QHBoxLayout;
        m_ritOnBtn = toggleBtn("RIT");
        m_ritOnBtn->setFixedWidth(36);
        m_ritOnBtn->setStyleSheet(ritActiveStyle);
        row->addWidget(m_ritOnBtn);

        m_ritMinus = new QPushButton("−");
        m_ritMinus->setFixedSize(22, 22);
        m_ritMinus->setFlat(true);
        row->addWidget(m_ritMinus);

        m_ritLabel = new QLabel("0 Hz");
        m_ritLabel->setAlignment(Qt::AlignCenter);
        m_ritLabel->setMinimumWidth(54);
        row->addWidget(m_ritLabel, 1);

        m_ritPlus = new QPushButton("+");
        m_ritPlus->setFixedSize(22, 22);
        m_ritPlus->setFlat(true);
        row->addWidget(m_ritPlus);

        m_ritClear = new QPushButton("CLR");
        m_ritClear->setFixedWidth(34);
        m_ritClear->setFlat(true);
        row->addWidget(m_ritClear);

        root->addLayout(row);

        connect(m_ritOnBtn, &QPushButton::toggled, this, [this](bool on) {
            if (m_slice) m_slice->setRit(on, m_slice->ritFreq());
        });
        connect(m_ritMinus, &QPushButton::clicked, this, [this] {
            if (!m_slice) return;
            m_slice->setRit(m_ritOnBtn->isChecked(), m_slice->ritFreq() - RIT_STEP_HZ);
        });
        connect(m_ritPlus, &QPushButton::clicked, this, [this] {
            if (!m_slice) return;
            m_slice->setRit(m_ritOnBtn->isChecked(), m_slice->ritFreq() + RIT_STEP_HZ);
        });
        connect(m_ritClear, &QPushButton::clicked, this, [this] {
            if (!m_slice) return;
            m_slice->setRit(m_ritOnBtn->isChecked(), 0);
        });
    }

    // ── XIT ───────────────────────────────────────────────────────────────────
    {
        auto* row = new QHBoxLayout;
        m_xitOnBtn = toggleBtn("XIT");
        m_xitOnBtn->setFixedWidth(36);
        m_xitOnBtn->setStyleSheet(ritActiveStyle);
        row->addWidget(m_xitOnBtn);

        m_xitMinus = new QPushButton("−");
        m_xitMinus->setFixedSize(22, 22);
        m_xitMinus->setFlat(true);
        row->addWidget(m_xitMinus);

        m_xitLabel = new QLabel("0 Hz");
        m_xitLabel->setAlignment(Qt::AlignCenter);
        m_xitLabel->setMinimumWidth(54);
        row->addWidget(m_xitLabel, 1);

        m_xitPlus = new QPushButton("+");
        m_xitPlus->setFixedSize(22, 22);
        m_xitPlus->setFlat(true);
        row->addWidget(m_xitPlus);

        m_xitClear = new QPushButton("CLR");
        m_xitClear->setFixedWidth(34);
        m_xitClear->setFlat(true);
        row->addWidget(m_xitClear);

        root->addLayout(row);

        connect(m_xitOnBtn, &QPushButton::toggled, this, [this](bool on) {
            if (m_slice) m_slice->setXit(on, m_slice->xitFreq());
        });
        connect(m_xitMinus, &QPushButton::clicked, this, [this] {
            if (!m_slice) return;
            m_slice->setXit(m_xitOnBtn->isChecked(), m_slice->xitFreq() - RIT_STEP_HZ);
        });
        connect(m_xitPlus, &QPushButton::clicked, this, [this] {
            if (!m_slice) return;
            m_slice->setXit(m_xitOnBtn->isChecked(), m_slice->xitFreq() + RIT_STEP_HZ);
        });
        connect(m_xitClear, &QPushButton::clicked, this, [this] {
            if (!m_slice) return;
            m_slice->setXit(m_xitOnBtn->isChecked(), 0);
        });
    }

    root->addStretch();
}

// ─── Slice wiring ─────────────────────────────────────────────────────────────

void RxApplet::setSlice(SliceModel* slice)
{
    if (m_slice) disconnectSlice(m_slice);
    m_slice = slice;
    if (m_slice) connectSlice(m_slice);
}

void RxApplet::connectSlice(SliceModel* s)
{
    // Sync all state from the model immediately
    // Antenna
    {
        const bool ant1 = (s->rxAntenna() == "ANT1");
        m_antBtns[0]->setChecked(ant1);
        m_antBtns[1]->setChecked(!ant1);
    }
    connect(s, &SliceModel::rxAntennaChanged, this, [this](const QString& ant) {
        m_antBtns[0]->setChecked(ant == "ANT1");
        m_antBtns[1]->setChecked(ant == "ANT2");
    });

    // Filter
    connect(s, &SliceModel::filterChanged, this, [this](int, int) {
        updateFilterButtons();
    });
    updateFilterButtons();

    // AGC
    connect(s, &SliceModel::agcModeChanged, this, [this](const QString&) {
        updateAgcButtons();
    });
    updateAgcButtons();

    // Squelch
    {
        m_sqlBtn->setChecked(s->squelchOn());
        m_sqlSlider->setValue(s->squelchLevel());
        m_sqlLabel->setText(QString::number(s->squelchLevel()));
    }
    connect(s, &SliceModel::squelchChanged, this, [this](bool on, int level) {
        QSignalBlocker b1(m_sqlBtn), b2(m_sqlSlider);
        m_sqlBtn->setChecked(on);
        m_sqlSlider->setValue(level);
        m_sqlLabel->setText(QString::number(level));
    });

    // DSP
    {
        QSignalBlocker b1(m_nbBtn), b2(m_nrBtn), b3(m_anfBtn);
        m_nbBtn->setChecked(s->nbOn());
        m_nrBtn->setChecked(s->nrOn());
        m_anfBtn->setChecked(s->anfOn());
    }
    connect(s, &SliceModel::nbChanged,  this, [this](bool on) { QSignalBlocker b(m_nbBtn);  m_nbBtn->setChecked(on); });
    connect(s, &SliceModel::nrChanged,  this, [this](bool on) { QSignalBlocker b(m_nrBtn);  m_nrBtn->setChecked(on); });
    connect(s, &SliceModel::anfChanged, this, [this](bool on) { QSignalBlocker b(m_anfBtn); m_anfBtn->setChecked(on); });

    // RIT
    {
        QSignalBlocker b(m_ritOnBtn);
        m_ritOnBtn->setChecked(s->ritOn());
        m_ritLabel->setText(QString("%1 Hz").arg(s->ritFreq()));
    }
    connect(s, &SliceModel::ritChanged, this, [this](bool on, int hz) {
        QSignalBlocker b(m_ritOnBtn);
        m_ritOnBtn->setChecked(on);
        m_ritLabel->setText(QString("%1 Hz").arg(hz));
    });

    // XIT
    {
        QSignalBlocker b(m_xitOnBtn);
        m_xitOnBtn->setChecked(s->xitOn());
        m_xitLabel->setText(QString("%1 Hz").arg(s->xitFreq()));
    }
    connect(s, &SliceModel::xitChanged, this, [this](bool on, int hz) {
        QSignalBlocker b(m_xitOnBtn);
        m_xitOnBtn->setChecked(on);
        m_xitLabel->setText(QString("%1 Hz").arg(hz));
    });
}

void RxApplet::disconnectSlice(SliceModel* s)
{
    s->disconnect(this);
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

void RxApplet::applyFilterPreset(int widthHz)
{
    if (!m_slice) return;

    int lo, hi;
    const QString& mode = m_slice->mode();

    if (mode == "LSB" || mode == "DIGL" || mode == "CWL") {
        lo = -widthHz;
        hi = 0;
    } else if (mode == "CW") {
        // CW: filter centred 200 Hz above carrier (typical 600 Hz pitch)
        lo = 200;
        hi = 200 + widthHz;
    } else {
        // USB, DIGU, AM, FM, DIG, etc.
        lo = 0;
        hi = widthHz;
    }

    m_slice->setFilterWidth(lo, hi);
}

void RxApplet::updateFilterButtons()
{
    if (!m_slice) {
        for (auto* b : m_filterBtns) { QSignalBlocker sb(b); b->setChecked(false); }
        return;
    }
    const int width = m_slice->filterHigh() - m_slice->filterLow();
    for (int i = 0; i < 6; ++i) {
        QSignalBlocker sb(m_filterBtns[i]);
        m_filterBtns[i]->setChecked(std::abs(width - FILTER_WIDTHS[i]) <= 150);
    }
}

void RxApplet::updateAgcButtons()
{
    const QString cur = m_slice ? m_slice->agcMode() : "";
    for (int i = 0; i < 4; ++i) {
        QSignalBlocker sb(m_agcBtns[i]);
        m_agcBtns[i]->setChecked(cur == AGC_MODES[i]);
    }
}

} // namespace AetherSDR
