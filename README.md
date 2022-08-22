# gifu-creation-koubou-teaching-material
こちらは、オンラインワークショップ 岐阜クリエーション工房2022 2. 自分の電子楽器をデザインしよう！
https://sites.google.com/iamas.ac.jp/gifu-creation-workshop/2022?authuser=0#h.r4l3rx4b2rmy
で配布した教材のリポジトリです。

## 教材基板ハードウェアの設計図(pdf)
- [ベースボードの回路図](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/hardware/base-pcb/gifu-creation-koubou-teaching-material-schematics.pdf)
- [ベースボードの基板](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/hardware/base-pcb/gifu-creation-koubou-teaching-material-pcb.pdf)

- [フィルターの回路図](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/hardware/vcf-13700/vcf-13700-schematics.pdf)
- [フィルターの基板](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/hardware/vcf-13700/vcf-13700-pcb.pdf)

- [ディレイの回路図](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/hardware/PT2399delay/PT2399delay-schematics.pdf)
- [ディレイの基板](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/hardware/PT2399delay/PT2399delay-pcb.pdf)

## マイコンのソフト
[Platform IO](https://platformio.org/) という開発環境を使っています。
- [ライブラリを含んだ全体](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/tree/main/software/gifu-creation-koubou-teathing-material-esp32)
- [一番重要な内容が書いてあるファイル](https://github.com/kinoshita-lab/gifu-creation-koubou-teaching-material/blob/main/software/gifu-creation-koubou-teathing-material-esp32/src/main.cpp)
