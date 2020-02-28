# 帯状シームによる縮小サムシング

※工事中．

## belt-like_seam_and_seamcarving
帯状シームによる画像の縮小とシームカービングによる画像の縮小を行うsourceです

linuxで下記のコマンドを入力します

    make
    sh run.sh

requirements

    mkdir resize/
    mkdir seam_band_image/
    mkdir resize_process/resize/(入力画像のファイル名)/
    mkdir resize_process/seam/(入力画像のファイル名)/

makeやshの使い方については有識者に問い合わせをお願いします


・new_smc.h
   
classや汎用関数の定義(common名前空間)

<br>

・resize_operator.cpp
    
seamcarving関数で縮小処理を行う

大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック

<br>
headerも含めて800行無いので解説無し．（ある程度C言語の知識があれば，1週間で8割読めると踏んでます）

以下，実装に関してのPoint

* 垂直方向のパス探索のみ関数を定義し，水平方向の探索は転置を行うことで実現

* 画面両端をシームとして選択することを禁止している（本当は選択するべき．理由は画面外を選択しないようにする配慮（過去の先輩談）．どなたか実装お願いします）

* 帯状シームは固定長のため，帯の左端の画素を代表画素とすることで，シームカービングと同様の関数で取り扱うことができる

* namespaceを事前定義しない主義のため，デバッグ作業やSTL使用のときは注意

-----------------


## Multi-Operator
帯状シームによる画像の縮小と線形スケーリングの併用による画像の縮小を行うsourceです．

(※注) オブジェクトマスクを利用するため，縮小対象画像等はsampleディレクトリに入れてます．

linuxで下記のコマンドを入力します

    make
    sh run.sh

requirements

    mkdir resize/uniform_ratio
    mkdir resize/scaling/


・new_smc.h
   
belt-like_seam_and_seamcarvingとほぼ同様．bilateral filterの実装が追加されている．

<br>

・Multi-Operator.cpp
    
帯状シームによりアスペクト比を揃えつつ縮小．判定によりスケーリングに切り替え．

大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック

<br>
帯状シームからの追加コードが200行にも満たないのでこちらも解説無し．

以下，実装に関してのPoint

* オブジェクトマスクは[Mask R-CNN](https://github.com/matterport/Mask_RCNN)を使用して作成．

* マスク内のオブジェクトの個数の数え上げ，オブジェクト毎にマスクを自動的に作成するのに一苦労．main関数内で以下の処理を行っている．

    1．オブジェクト毎に色がついてるため，色数を数える．<br>
    2．オブジェクトの数をparam->color_numに格納して，マスクと残存率，判定用重みのコンテナを個数分作成．<br>
    3．tupleとcolor_numを駆使して，マスクを作成．<br>

* filter parameterは画像ごとに最適化した結果を使用している．(誤ってparameterを最適化するためのprogramはdeleteしてしまいました．Multi-Operatorのプログラムを少し改造すると作成できます．詳細は以下参照)

* 帯状シームで削り終わった画像はresize/uniform_ratioに，スケーリング後の画像はresize/scalingに格納．
