#include "anthyplugin.h"
#include <QDebug>
#include <QtQml>

// ========== AnthyEngine ==========

AnthyEngine::AnthyEngine(QObject *parent)
    : QObject(parent)
    , m_context(nullptr)
    , m_initialized(false)
{
    // Anthy 初期化
    if (anthy_init() == 0) {
        m_context = anthy_create_context();
        if (m_context) {
            // UTF-8 エンコーディングを設定
            anthy_context_set_encoding(m_context, ANTHY_UTF8_ENCODING);
            m_initialized = true;
            qDebug() << "AnthyEngine: initialized successfully";
        } else {
            qWarning() << "AnthyEngine: failed to create context";
        }
    } else {
        qWarning() << "AnthyEngine: failed to initialize anthy";
    }
}

AnthyEngine::~AnthyEngine()
{
    if (m_context) {
        anthy_release_context(m_context);
    }
    anthy_quit();
}

QString AnthyEngine::preedit() const
{
    return m_preedit;
}

void AnthyEngine::setPreedit(const QString &text)
{
    if (m_preedit != text) {
        m_preedit = text;
        emit preeditChanged();
    }
}

QVariantList AnthyEngine::segments() const
{
    return m_segments;
}

bool AnthyEngine::convert(const QString &hiragana)
{
    if (!m_initialized || !m_context) {
        qWarning() << "AnthyEngine: not initialized";
        return false;
    }

    // ひらがなを設定
    QByteArray utf8 = hiragana.toUtf8();
    int result = anthy_set_string(m_context, utf8.constData());
    
    if (result < 0) {
        qWarning() << "AnthyEngine: anthy_set_string failed";
        return false;
    }

    m_preedit = hiragana;
    updateSegments();
    
    emit preeditChanged();
    emit segmentsChanged();
    
    return true;
}

void AnthyEngine::updateSegments()
{
    m_segments.clear();
    m_selectedCandidates.clear();

    if (!m_context) return;

    struct anthy_conv_stat stat;
    if (anthy_get_stat(m_context, &stat) < 0) {
        return;
    }

    char buf[1024];
    
    for (int i = 0; i < stat.nr_segment; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat(m_context, i, &seg_stat);

        QVariantMap segment;
        segment["index"] = i;
        segment["candidateCount"] = seg_stat.nr_candidate;
        segment["length"] = seg_stat.seg_len;

        // 最初の候補（最有力候補）を取得
        if (anthy_get_segment(m_context, i, 0, buf, sizeof(buf)) > 0) {
            segment["text"] = QString::fromUtf8(buf);
        }

        m_segments.append(segment);
        m_selectedCandidates.append(0);
    }
}

QStringList AnthyEngine::getCandidates(int segmentIndex)
{
    QStringList candidates;
    
    if (!m_context || segmentIndex < 0 || segmentIndex >= m_segments.size()) {
        return candidates;
    }

    struct anthy_segment_stat seg_stat;
    if (anthy_get_segment_stat(m_context, segmentIndex, &seg_stat) < 0) {
        return candidates;
    }

    char buf[1024];
    for (int i = 0; i < seg_stat.nr_candidate; i++) {
        if (anthy_get_segment(m_context, segmentIndex, i, buf, sizeof(buf)) > 0) {
            candidates.append(QString::fromUtf8(buf));
        }
    }

    return candidates;
}

QString AnthyEngine::getCandidate(int segmentIndex, int candidateIndex)
{
    if (!m_context) return QString();

    char buf[1024];
    if (anthy_get_segment(m_context, segmentIndex, candidateIndex, buf, sizeof(buf)) > 0) {
        return QString::fromUtf8(buf);
    }
    return QString();
}

bool AnthyEngine::selectCandidate(int segmentIndex, int candidateIndex)
{
    if (!m_context || segmentIndex < 0 || segmentIndex >= m_selectedCandidates.size()) {
        return false;
    }

    m_selectedCandidates[segmentIndex] = candidateIndex;

    // segments を更新
    if (segmentIndex < m_segments.size()) {
        QVariantMap segment = m_segments[segmentIndex].toMap();
        segment["text"] = getCandidate(segmentIndex, candidateIndex);
        m_segments[segmentIndex] = segment;
        emit segmentsChanged();
    }

    return true;
}

void AnthyEngine::resizeSegment(int segmentIndex, int delta)
{
    if (!m_context) return;
    
    anthy_resize_segment(m_context, segmentIndex, delta);
    updateSegments();
    emit segmentsChanged();
}

QString AnthyEngine::commit()
{
    QString result;

    if (!m_context) return result;

    // 選択された候補を結合
    for (int i = 0; i < m_segments.size(); i++) {
        int candidateIndex = (i < m_selectedCandidates.size()) ? m_selectedCandidates[i] : 0;
        result += getCandidate(i, candidateIndex);
        anthy_commit_segment(m_context, i, candidateIndex);
    }

    emit conversionResult(result);
    reset();
    
    return result;
}

void AnthyEngine::reset()
{
    if (m_context) {
        anthy_reset_context(m_context);
    }
    m_preedit.clear();
    m_segments.clear();
    m_selectedCandidates.clear();
    
    emit preeditChanged();
    emit segmentsChanged();
}

QStringList AnthyEngine::predict(const QString &hiragana)
{
    QStringList predictions;

    if (!m_context) return predictions;

    QByteArray utf8 = hiragana.toUtf8();
    if (anthy_set_prediction_string(m_context, utf8.constData()) < 0) {
        return predictions;
    }

    struct anthy_prediction_stat stat;
    if (anthy_get_prediction_stat(m_context, &stat) < 0) {
        return predictions;
    }

    char buf[1024];
    for (int i = 0; i < stat.nr_prediction && i < 10; i++) {
        if (anthy_get_prediction(m_context, i, buf, sizeof(buf)) > 0) {
            predictions.append(QString::fromUtf8(buf));
        }
    }

    return predictions;
}

// ========== AnthyPlugin ==========

void AnthyPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<AnthyEngine>(uri, 1, 0, "AnthyEngine");
}
