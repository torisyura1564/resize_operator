#! /bin/sh

dataset="kodak"
#dataset="voc2012"

weight="1"

for seam in 0
do

#for img in 6 11 12 16 19 22 23 25 27 28 29
for img in 19
do

 if   [ "$seam" = 0 ]; then SEAMCARVING="OFF"
 elif [ "$seam" = 1 ]; then SEAMCARVING="ON"
 fi

 ## IMAGE ##
 if   [ "$dataset" = "kodak" ]; then
	  if   [ "$img" = 1 ]; then NAME="01"
	  elif [ "$img" = 2 ]; then NAME="02"
	  elif [ "$img" = 3 ]; then NAME="03"
	  elif [ "$img" = 4 ]; then NAME="04"
	  elif [ "$img" = 5 ]; then NAME="05"
	  elif [ "$img" = 6 ]; then NAME="06"
	  elif [ "$img" = 7 ]; then NAME="07"
	  elif [ "$img" = 8 ]; then NAME="08"
	  elif [ "$img" = 9 ]; then NAME="09"
	  elif [ "$img" = 10 ]; then NAME="10"
	  elif [ "$img" = 11 ]; then NAME="11"
	  elif [ "$img" = 12 ]; then NAME="12"
	  elif [ "$img" = 13 ]; then NAME="13"
	  elif [ "$img" = 14 ]; then NAME="14"
	  elif [ "$img" = 15 ]; then NAME="15"
	  elif [ "$img" = 16 ]; then NAME="16"
	  elif [ "$img" = 17 ]; then NAME="17"
	  elif [ "$img" = 18 ]; then NAME="18"
	  elif [ "$img" = 19 ]; then NAME="19"
	  elif [ "$img" = 20 ]; then NAME="20"
	  elif [ "$img" = 21 ]; then NAME="21"
	  elif [ "$img" = 22 ]; then NAME="22"
	  elif [ "$img" = 23 ]; then NAME="23"
	  elif [ "$img" = 24 ]; then NAME="24"
	  fi
  elif [ "$dataset" = "voc2012" ]; then
	  if   [ "$img" = 1 ]; then NAME="2007_000068"
	  elif [ "$img" = 2 ]; then NAME="2007_000175"
	  elif [ "$img" = 3 ]; then NAME="2007_000256"
	  elif [ "$img" = 4 ]; then NAME="2007_000464"
	  elif [ "$img" = 5 ]; then NAME="2008_005938"
	  elif [ "$img" = 6 ]; then NAME="2007_008415"
	  elif [ "$img" = 7 ]; then NAME="2007_009323"
	  elif [ "$img" = 8 ]; then NAME="2007_002954"
	  elif [ "$img" = 9 ]; then NAME="2007_000925"
	  fi
  fi

 ## IMAGE ##
  if   [ "$dataset" = "kodak" ]; then
	  if   [ "$img" = 1 ]; then X="0"; Y="40"
	  elif [ "$img" = 2 ]; then X="0"; Y="40"
	  elif [ "$img" = 3 ]; then X="0"; Y="40"
	  elif [ "$img" = 4 ]; then X="0"; Y="40"
	  elif [ "$img" = 5 ]; then X="0"; Y="40"
	  elif [ "$img" = 6 ]; then X="40"; Y="0"
	  elif [ "$img" = 7 ]; then X="0"; Y="40"
	  elif [ "$img" = 8 ]; then X="0"; Y="40"
	  elif [ "$img" = 9 ]; then X="0"; Y="40"
	  elif [ "$img" = 10 ]; then X="0"; Y="40"
	  elif [ "$img" = 11 ]; then X="80"; Y="0"
	  elif [ "$img" = 12 ]; then X="100"; Y="232"
	  elif [ "$img" = 13 ]; then X="120"; Y="80"
	  elif [ "$img" = 14 ]; then X="0"; Y="40"
	  elif [ "$img" = 15 ]; then X="0"; Y="40"
	  elif [ "$img" = 16 ]; then X="200"; Y="0"
	  elif [ "$img" = 17 ]; then X="0"; Y="40"
	  elif [ "$img" = 18 ]; then X="0"; Y="40"
	  elif [ "$img" = 19 ]; then X="0"; Y="40"
	  elif [ "$img" = 20 ]; then X="60"; Y="40"
	  elif [ "$img" = 21 ]; then X="0"; Y="40"
	  elif [ "$img" = 22 ]; then X="60"; Y="80"
	  elif [ "$img" = 23 ]; then X="0"; Y="160"
	  elif [ "$img" = 24 ]; then X="0"; Y="40"
	  fi
  elif [ "$dataset" = "voc2012" ]; then
	  if   [ "$img" = 1 ]; then X="0"; Y="160"
	  elif [ "$img" = 2 ]; then X="0"; Y="120"
	  elif [ "$img" = 3 ]; then X="148"; Y="0"
	  elif [ "$img" = 4 ]; then X="240"; Y="0"
	  elif [ "$img" = 5 ]; then X="0"; Y="100"
	  elif [ "$img" = 6 ]; then X="0"; Y="200"
	  elif [ "$img" = 7 ]; then X="0"; Y="100"
	  elif [ "$img" = 8 ]; then X="0"; Y="100"
	  elif [ "$img" = 9 ]; then X="152"; Y="300"
	  fi
  fi

 ## WEIGHT ##
  if   [ "$weight" = 1 ]; then COST="FE_RE"; F="1"; R="1";
  elif [ "$weight" = 2 ]; then COST="FE"; F="1"; R="0";
  elif [ "$weight" = 3 ]; then COST="RE"; F="0"; R="1";
  fi

 ALL_SEAM="x_$X,y_$Y"

 KODAK_GRAY="/rda2/DATABASE/kodak/gray/"
 KODAK_COLOR="/rda2/DATABASE/kodak/color/"
 VOC2012="/rda2/DATABASE/VOCdevkit/VOC2012/JPEGImages/"

 KODAK_GRAY_IMAGE=$KODAK_GRAY$NAME".pgm"
 KODAK_COLOR_IMAGE=$KODAK_COLOR$NAME".ppm"
 VOC2012_IMAGE=$VOC2012$NAME".jpg"

 if   [ "$dataset" = "kodak" ]; then TEST=$KODAK_COLOR_IMAGE
 elif [ "$dataset" = "voc2012" ]; then TEST=$VOC2012_IMAGE
 fi

 if [ "$SEAMCARVING" = "OFF" ]; then FLAG="-x $X -y $Y -f $F -r $R -B 16 -C 4 -S 0"
   ./MKSMC $FLAG $NAME $TEST $COST $ALL_SEAM
 fi

 if [ "$SEAMCARVING" = "ON" ]; then FLAG="-x $X -y $Y -f $F -r $R -B 0 -C 1 -S 1"
   ./MKSMC $FLAG $NAME $TEST $COST $ALL_SEAM
 fi

done
done