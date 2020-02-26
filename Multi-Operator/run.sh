#! /bin/sh2
#for img in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
for img in 10
do

for surv in 0.9
do

weight="1"

for FILTER in "ON" "OFF"
do

## IMAGE ##
  if   [ "$img" = 1 ]; then NAME="2007_000068"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 2 ]; then NAME="2007_000175"; kernel="7"; sigma_color="60.0"; sigma_space="3.5"
  elif [ "$img" = 3 ]; then NAME="2007_000256"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 4 ]; then NAME="2007_000464"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 5 ]; then NAME="2007_000925"; kernel="7"; sigma_color="60.0"; sigma_space="3.5"
  elif [ "$img" = 6 ]; then NAME="2007_001288"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 7 ]; then NAME="2007_002954"; kernel="9"; sigma_color="60.0"; sigma_space="1.5"
  elif [ "$img" = 8 ]; then NAME="2007_005130"; kernel="7"; sigma_color="60.0"; sigma_space="3.5"
  elif [ "$img" = 9 ]; then NAME="2007_006899"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 10 ]; then NAME="2007_008415"; kernel="11"; sigma_color="60.0"; sigma_space="3.5"
  elif [ "$img" = 11 ]; then NAME="2007_009082"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 12 ]; then NAME="2007_009323"; kernel="5"; sigma_color="60.0"; sigma_space="2.5"
  elif [ "$img" = 13 ]; then NAME="2007_009346"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 14 ]; then NAME="2008_005938"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 15 ]; then NAME="2008_008268"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 16 ]; then NAME="2009_000664"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 17 ]; then NAME="2009_001385"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 18 ]; then NAME="2009_002549"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 19 ]; then NAME="2009_003666"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 20 ]; then NAME="2010_001768"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 21 ]; then NAME="2010_001908"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 22 ]; then NAME="2010_004994"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 23 ]; then NAME="2010_005519"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 24 ]; then NAME="2011_000069"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 25 ]; then NAME="2011_000834"; kernel="5"; sigma_color="50.0"; sigma_space="2.5"
  elif [ "$img" = 26 ]; then NAME="2007_001073"; kernel="5"; sigma_color="60.0"; sigma_space="2.5"
  elif [ "$img" = 27 ]; then NAME="2007_004830"; kernel="5"; sigma_color="60.0"; sigma_space="2.5"
  elif [ "$img" = 28 ]; then NAME="06"; kernel="5"; sigma_color="60.0"; sigma_space="2.5"
  elif [ "$img" = 29 ]; then NAME="11"; kernel="5"; sigma_color="60.0"; sigma_space="2.5"
  fi

 ## WEIGHT ##
 if   [ "$weight" = 1 ]; then COST="FE_RE"
 elif [ "$weight" = 2 ]; then COST="FE"
 elif [ "$weight" = 3 ]; then COST="RE"
 fi

 ## WEIGHT ##
 if   [ "$weight" = 1 ]; then F="1"; R="1";
 elif [ "$weight" = 2 ]; then F="1"; R="0";
 elif [ "$weight" = 3 ]; then F="0"; R="1";
 fi

 ## IMAGE (Size, Aspect ratio) ##
  if   [ "$img" = 1 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 2 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 3 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 4 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 5 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 6 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 7 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 8 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 9 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 10 ]; then X="300"; Y="150"; R_X="2"; R_Y="1"
  elif [ "$img" = 11 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 12 ]; then X="200"; Y="300"; R_X="2"; R_Y="3"
  elif [ "$img" = 13 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 14 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 15 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 16 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 17 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 18 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 19 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 20 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 21 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 22 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 23 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 24 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 25 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 26 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 27 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 28 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  elif [ "$img" = 29 ]; then X="200"; Y="200"; R_X="1"; R_Y="1"
  fi
  
  ## surv_rate list ##
  S_R=$surv
  S_G=$surv
  S_B=$surv
  S_P=$surv
  S_Y=$surv  
  S_LG=$surv
  S_GR=$surv
  S_O=$surv
  S_YG=$surv

  ALL_SEAM="ratio_"$R_X","$R_Y

  COLOR_IMAGE="../sample/ppmimages/$NAME.ppm"
  MASK_IMAGE="../sample/maskimages2/$NAME.png"

  if [ "$FILTER" = "OFF" ]; then 
	FLAG="-x $R_X -y $R_Y -f $F -r $R -b 16 -c 4 -k 0 -C 0.0 -S 0.0";
	LAST_IMAGE="./resize/uniform_ratio/"$NAME"_"$COST"_"$ALL_SEAM".ppm";
	LASTNAME="scaling_"$NAME"_"$X"_"$Y
  fi

  if [ "$FILTER" = "ON"  ]; then
	FLAG="-x $R_X -y $R_Y -f $F -r $R -b 16 -c 4 -k $kernel -C $sigma_color -S $sigma_space";
	LAST_IMAGE="./resize/uniform_ratio/"$NAME"_"$COST"_"$ALL_SEAM"_"$kernel"_"$sigma_color"_"$sigma_space".ppm";
	LASTNAME="scaling_filtered_"$NAME"_"$X"_"$Y
  fi

  FLAG_RATE="$S_R $S_G $S_B $S_P $S_Y $S_LG $S_GR $S_O $S_YG"

  ./MKSMC $FLAG $FLAG_RATE $NAME $COLOR_IMAGE $MASK_IMAGE $COST $ALL_SEAM
  ./MKSCL $LAST_IMAGE $LASTNAME $X $Y


done
done
done