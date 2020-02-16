# belt-like_seam_and_seamcarving

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

-----------------
・new_smc.h
   
classや汎用関数の定義(common名前空間)

<br>

・resize_operator.cpp
    
seamcarving関数で縮小処理を行う

大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック

headerも含めて800行無いので解説無し．（ある程度C言語の知識があれば，1週間で8割読めると踏んでます）

以下，実装に関してのPoint

1．垂直方向のパス探索のみ関数を定義し，水平方向の探索は転置を行うことで実現

2．画面両端をシームとして選択することを禁止している（本当は選択するべき．理由は画面外を選択しないようにする配慮（過去の先輩談）．どなたか実装お願いします）

3．帯状シームは固定長のため，帯の左端の画素を代表画素とすることで，シームカービングと同様の関数で取り扱うことができる

4．namespaceを事前定義しない主義のため，デバッグ作業やSTL使用のときは注意

-----------------
