# 帯状シームによる縮小サムシング

※工事中．

## belt-like_seam_and_seamcarving
大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック

### abstract
帯状シームによる画像の縮小&シームカービングを実装したsourceです  
処理が軽い方(?)なので，自分のPCでも回せます． Visual Studioと計算機での回し方について以下記します．

### Visual Studio編
#### requirements  
    ・Visual Studio 2015以上  
    ・opencv-3.x (opencv-4.xは定数マクロに名前空間が付与するため，少々の変更要)  

#### Visual Studioのインストール  
大学のライセンスでEnterprise(?)がインストールできるかと思いますが，私はCommunityでやってました．  
Visual Studioでアプリ開発，サーバー構築が出来るためワークロードをどうするかという項目がありますが，特にチェックせずにinstall．  

#### opencvのインストール
次のリンク[opencvをgithubからinstall](https://github.com/opencv/opencv/releases) から好きなversionの **.exeファイル** ファイルをダウンロード．  
exeを起動し，好みの場所に解凍．(PATHへの追加しやすさから，Cドライブ直下に置くと良し)    
環境変数のPATHに'C:\opencv\build\bin'と'C:\opencv\build\x64\vc14(15でもおｋ)\bin'を追加  

#### プログラム動作への準備  
1．Visual Studioを起動し，ファイル→新規作成→プロジェクトをクリックし，空のプロジェクトを選択して適当なプロジェクト名で作成．  
2．C:\ユーザー\(自分のユーザー名)\source\repos にプロジェクトが作成される．  
3．Visual Studio上でソースファイルに.cppファイルをドラッグ．ヘッダファイルに.hファイルをドラッグ  
  (ドラッグするファイルをrepos\(プロジェクト名) に置いても良い)  
4．次のリンク[opencv環境構築](https://blog.tukuyo.net/entry/2018/10/04/233140)を参考にライブラリをプロパティに設定  
  (Debag modeだと遅いため，Release推奨)  
5．プロパティ内のリンカー→システム→サブシステムをコンソールに設定  
6．.cppのmain関数先頭にあるforループを変更する． (コマンドライン引数の設定が使えない子のため) 　
7．repos\(プロジェクト名)に以下を追加

    mkdir resize/
    mkdir seam_band_image/
    mkdir resize_process/resize/(入力画像のファイル名)/
    mkdir resize_process/seam/(入力画像のファイル名)/


#### 実行  
ビルド→デバッグなしで開始  

### Linux編
以下のコマンドを入力することで実行可能．makeの後エラー文の出力が無くMKSMCファイルが出力されていることを確認．

    cd belt-like_seam_and_seamcarving
    make
    sh run.sh

requirements

    mkdir resize/
    mkdir seam_band_image/
    mkdir resize_process/resize/(入力画像のファイル名)/
    mkdir resize_process/seam/(入力画像のファイル名)/

#### make
- コンパイルを行うコマンド(複数ファイルをまとめて実行ファイルにしたいときに便利)
- makeの流れは以下の図を参照。

<div align="center">
<img src="https://brain.cc.kogakuin.ac.jp/~kanamaru/lecture/prog1/01/prog-flow.png" alt="属性" title="makeの流れ">
</div>

<div align="center">
    <a href="https://brain.cc.kogakuin.ac.jp/~kanamaru/lecture/prog1/02-intro01.html">引用サイト(https://brain.cc.kogakuin.ac.jp/~kanamaru/lecture/prog1/02-intro01.html)</a>
    <br>
(上図は単一のcppをmakeした場合．複数cppをmakeしたい場合はそれぞれの.oを作成した後にリンクで結合．)
</div>



### ファイル詳細    
#### new_smc.h
・classや汎用関数の定義(common名前空間)  

#### resize_operator.cpp
・seamcarving関数で縮小処理を行う

headerも含めて800行無いので解説無し．（ある程度C言語の知識があれば，1週間で8割読めると踏んでます）

以下，実装に関してのPoint

* 垂直方向のパス探索のみ関数を定義し，水平方向の探索は転置を行うことで実現  
* 画面両端をシームとして選択することを禁止している（本当は選択するべき．理由は画面外を選択しないようにする配慮（過去の先輩談）．どなたか実装お願いします）  
* 帯状シームは固定長のため，帯の左端の画素を代表画素とすることで，シームカービングと同様の関数で取り扱うことができる  
* namespaceを事前定義しない主義のため，デバッグ作業やSTL使用のときは注意  

-----------------


## Multi-Operator

### abstract
帯状シームによる縮小と線形スケーリングの併用による画像の縮小を行うsourceです．  
(※注) オブジェクトマスクを利用するため，縮小対象画像等はsampleディレクトリに入れてます．  
動作方法はbelt-like_seam_and_seamcarvingと同様のため，割愛．　　
大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック  

### requirements

    mkdir resize/uniform_ratio
    mkdir resize/scaling/


### ファイル詳細
#### new_smc.h
belt-like_seam_and_seamcarvingとほぼ同様．エッジ保持平滑化フィルタのBilateralFilterの実装が追加されている． 
(opencvの実装を使わない理由は色成分を量子化したり，カーネルの範囲が正方形になってなかったりしてるため)  


#### Multi-Operator.cpp
帯状シームによりアスペクト比を揃えつつ縮小．判定によりスケーリングに切り替え．  
帯状シームからの追加コードが200行にも満たないのでこちらも解説無し．  

以下，実装に関してのPoint

* オブジェクトマスクは[Mask R-CNN](https://github.com/matterport/Mask_RCNN)を使用して作成．

* マスク内のオブジェクトの個数の数え上げ，オブジェクト毎にマスクを自動的に作成するのに一苦労．main関数内で以下の処理を行っている．

    1．オブジェクト毎に色がついてるため，色数を数える．<br>
    2．オブジェクトの数をparam->color_numに格納して，マスクと残存率，判定用重みのコンテナを個数分作成．<br>
    3．tupleとcolor_numを駆使して，マスクを作成．<br>

* filter parameterは画像ごとに最適化した結果を使用している．(誤ってparameterを最適化するためのprogramはdeleteしてしまいました．Multi-Operatorのプログラムを少し改造すると作成できます．詳細は以下参照)

* 帯状シームで削り終わった画像はresize/uniform_ratioに，スケーリング後の画像はresize/scalingに格納．
