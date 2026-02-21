#ifndef ANTHYPLUGIN_H
#define ANTHYPLUGIN_H

#include <QObject>
#include <QQmlExtensionPlugin>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

extern "C" {
#include <anthy/anthy.h>
}

// Anthy 変換エンジンラッパー
class AnthyEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString preedit READ preedit WRITE setPreedit NOTIFY preeditChanged)
    Q_PROPERTY(QVariantList segments READ segments NOTIFY segmentsChanged)

public:
    explicit AnthyEngine(QObject *parent = nullptr);
    ~AnthyEngine();

    QString preedit() const;
    void setPreedit(const QString &text);
    QVariantList segments() const;

    // 変換操作
    Q_INVOKABLE bool convert(const QString &hiragana);
    Q_INVOKABLE QStringList getCandidates(int segmentIndex);
    Q_INVOKABLE QString getCandidate(int segmentIndex, int candidateIndex);
    Q_INVOKABLE bool selectCandidate(int segmentIndex, int candidateIndex);
    Q_INVOKABLE void resizeSegment(int segmentIndex, int delta);
    Q_INVOKABLE QString commit();
    Q_INVOKABLE void reset();

    // 予測変換
    Q_INVOKABLE QStringList predict(const QString &hiragana);

signals:
    void preeditChanged();
    void segmentsChanged();
    void conversionResult(const QString &result);

private:
    void updateSegments();

    anthy_context_t m_context;
    QString m_preedit;
    QVariantList m_segments;
    QList<int> m_selectedCandidates;
    bool m_initialized;
};

// QML プラグイン登録
class AnthyPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif // ANTHYPLUGIN_H
