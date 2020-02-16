# belt-like_seam_and_seamcarving
resizing operator by belt-like seam and seamcarving

帯状シームによる画像の縮小とシームカービングによる画像の縮小を行うsourceです

linuxで下記のコマンドを入力します

    make
    sh run.sh

makeやshの使い方については有識者に問い合わせをお願いします
以下，sourceの説明

-----------------
・new_smc.h
   
classや汎用関数の定義(common名前空間)

-----------------
・resize_operator.cpp
    
seamcarving関数で縮小処理を行う

大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック

headerも含めて800行も無いので特に説明する項目は無し

-----------------
