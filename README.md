# belt-like_seam_and_seamcarving

帯状シームによる画像の縮小とシームカービングによる画像の縮小を行うsourceです

linuxで下記のコマンドを入力します

    make
    sh run.sh

同一ディレクトリに

・resize/
・seam_band_image/
・./resize_process/resize/(入力画像のファイル名)
・./resize_process/seam/(入力画像のファイル名)

が必要

makeやshの使い方については有識者に問い合わせをお願いします

-----------------
・new_smc.h
   
classや汎用関数の定義(common名前空間)

<br>

・resize_operator.cpp
    
seamcarving関数で縮小処理を行う

大まかな処理手順は2019年度修士2年 大和田の修論パワポをチェック

headerも含めて800行も無いので特に説明する項目は無し

-----------------
