# anthy-qml-plugin

Sailfish OS 向けの Anthy Unicode QML プラグイン。QML から直接 Anthy かな漢字変換エンジンを利用できます。

## 概要

このプラグインは、C 言語で書かれた Anthy Unicode ライブラリを QML から呼び出すためのブリッジです。日本語キーボードアプリなどで、ひらがなから漢字への変換機能を実装できます。

## 必要条件

- Sailfish OS 5.0 以降
- anthy-unicode ライブラリがインストールされていること
- Fedora 由来の辞書ファイル（anthy.dic）

## インストール

### 1. anthy-unicode のインストール

先に [anthy-unicode](https://github.com/fujiwarat/anthy-unicode) をビルドしてインストールしてください。

### 2. プラグインのビルド

```bash
cd /home/mersdk
git clone https://github.com/inugamine/anthy-qml-plugin.git
cd anthy-qml-plugin
mb2 -t SailfishOS-5.0.0.62EA-aarch64 build
```

### 3. 実機へのインストール

```bash
scp RPMS/anthy-qml-plugin-*.rpm defaultuser@<device-ip>:~/
ssh defaultuser@<device-ip>
devel-su rpm -i --nodeps anthy-qml-plugin-*.rpm
```

## 使い方

### QML での読み込み

```qml
import jp.anthy 1.0

AnthyEngine {
    id: anthy
    Component.onCompleted: {
        console.log("Anthy initialized")
    }
}
```

### 基本的な変換フロー

```qml
// 1. ひらがなを変換
if (anthy.convert("きょうはいいてんきです")) {
    // 2. 文節情報を取得
    console.log("文節数:", anthy.segments.length)
    
    for (var i = 0; i < anthy.segments.length; i++) {
        console.log("文節", i, ":", anthy.segments[i].text)
    }
    
    // 3. 候補を取得
    var candidates = anthy.getCandidates(0)  // 最初の文節
    console.log("候補:", candidates)
    
    // 4. 候補を選択
    anthy.selectCandidate(0, 1)  // 文節0の候補1を選択
    
    // 5. 確定
    var result = anthy.commit()
    console.log("確定:", result)
}

// リセット
anthy.reset()
```

## API リファレンス

### メソッド

| メソッド | 引数 | 戻り値 | 説明 |
|---------|------|--------|------|
| `convert(text)` | `string` | `bool` | ひらがなを変換。成功で true |
| `getCandidates(segmentIndex)` | `int` | `string[]` | 指定文節の変換候補リスト |
| `selectCandidate(segmentIndex, candidateIndex)` | `int, int` | `void` | 候補を選択 |
| `commit()` | - | `string` | 変換を確定して結果を返す |
| `reset()` | - | `void` | 変換状態をリセット |

### プロパティ

| プロパティ | 型 | 説明 |
|-----------|-----|------|
| `segments` | `list<object>` | 変換結果の文節リスト |

### segments オブジェクト

| プロパティ | 型 | 説明 |
|-----------|-----|------|
| `text` | `string` | 現在選択中の変換結果 |
| `length` | `int` | 元のひらがなの文字数 |

## 使用例：連文節変換

```qml
AnthyEngine {
    id: anthy
}

property int currentSegment: 0

function startConvert(hiragana) {
    if (anthy.convert(hiragana)) {
        currentSegment = 0
        showCandidates()
    }
}

function showCandidates() {
    var candidates = anthy.getCandidates(currentSegment)
    // UI に候補を表示
}

function selectAndNext(candidateIndex) {
    anthy.selectCandidate(currentSegment, candidateIndex)
    
    if (currentSegment < anthy.segments.length - 1) {
        // 次の文節へ
        currentSegment++
        showCandidates()
    } else {
        // 最後の文節なので確定
        var result = anthy.commit()
        // result を入力フィールドに送信
    }
}
```

## ファイル構成

```
anthy-qml-plugin/
├── anthyplugin.h        # AnthyEngine クラス定義
├── anthyplugin.cpp      # 実装
├── anthy-qml-plugin.pro # Qt プロジェクトファイル
├── qmldir               # QML モジュール定義
└── rpm/
    └── anthy-qml-plugin.spec  # RPM spec ファイル
```

## 注意事項

### 辞書ファイルについて

Anthy のソースからビルドした辞書では形態素解析が正しく動作しません。必ず **Fedora 由来の anthy.dic** を使用してください。

```bash
# Fedora パッケージから anthy.dic を取得して置き換え
devel-su cp anthy.dic /usr/share/anthy-unicode/anthy.dic
```

### アーキテクチャ

現在 **aarch64** のみ対応しています。i486 ビルドは anthy-unicode の mkdepgraph が segfault するため非対応です。

## ライセンス

GPL-3.0

## 関連プロジェクト

- [anthy-unicode](https://github.com/fujiwarat/anthy-unicode) - かな漢字変換エンジン本体
- [japanese-kana-kbd](https://github.com/inugamine/SailfishOS-Japanese-Keyboard) - Sailfish OS 日本語フリックキーボード

## 作者

- Ken (inugamine)

## 謝辞

- Anthy Unicode の開発者である fujiwarat さん
- Sailfish OS コミュニティ
