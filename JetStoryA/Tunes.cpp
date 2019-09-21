#include "JetStory.h"

const char *score_qsf[] = {
	"",
	R"( // QUIET
		#BRUM !174.61:L500V100A98D50S95R40w0:V90f0.50A98D30w0:f0.50A30D40S0w0:f5r-2w0:w0
		#SNARE !523.25:L500V75A98D50S0R40N:V20f0.25A98D30S0w0:V90f0.50A30D40S0w0:V90f5r-2w0:w0
		#DRUM !174.61:L200V80r-50A90D40R40:V130f7D45S0B:V90D55S0:f5r-2:
		
		!tempo:130 !volume:0 !volume_to:0.7:6

		#MELODY w 52795279 t2927292 52795279 t292 7292 749t 749t 0'4t49 4t4749t749t+0-4t494t492795245 q 2452 w t29t729t0'2t2 q 9t w 927692760'2t29269729t9262 m79 w 52795279t292729252795279t2927292749t749t0'4t494t4749t749t0'4t494t492795245 q 2452 w t29t729t0'2t2 q 9t w 927692760'2t29269729t9262 m79

		{ $SNARE ____ $200[/8 0_00 0_22 4444 000_] }
		{ $DRUM  ________ $200[q0] $22[q0] }

		[ $BRUM - $MELODY !cut ]
        
		!loop
		{ $SNARE $200[/8 0_00 0_22 4444 000_] }
		{ $DRUM  $200[q0] }
		{ $BRUM _ + $MELODY }
		[ $BRUM - $MELODY n_ !cut ]
	)",
	R"( // BASE
		#BIGDRUM !174.61:L200V100r-5A90D30R30:V130f7D45S0B:V90D55S0:f5r-2:
		#TRUMPET !391.99:L500V100A60D60S87R40w0:V100f0.50A98D50S0B:V90f0.50A30D40S0w0:V60f5r-2w0:V60f8w0
		#TOMTOM !174.61:L2000V100r-8D40S0R35:V90f3r-2D50S0B:V80r2D55S0N:f5r-2:
		
		$TRUMPET /4  @-5 0&4&7 /8 0&4&7 0&4&7 /4 2&5&9 5&9&0^ /2 +0&4&7-;
		$TOMTOM  /16 @-5 __7_0_7 2_59 9999 9 $BIGDRUM 0;
	)",
	R"( // BOSS
		#ORGAN !261.62:L250V65A70D40R40w1:L50V90f0.50A98R50w-4:V120D60S0B:V70f3w-4:w-4
		#TOMTOM !174.61:L2000V100r-8D40S0R35:V90f3r-2D50S0B:V80r2D55S0N:f5r-2:
		#PINGER !415.30:L250V100A98D40S0R40w3:L50V90f0.50A98R50w-4:V120D60S0B:w-4:w-4
		
		!loop !volume:0.85
		{ /8 $TOMTOM  -5757 5777 5757 5777 5757577757575777 5757 5777 5757 5777 5757577757575777 }
		{ /8 $TOMTOM  -/64_ /8 5757 5777 5757 57 /16 57 57 /8 5757 5777 5757 57 /16 57 57  /8 5757 5777 5757 57 /16 57 57 /8 5757 5777 5757 57 /16 57 57 }
		{ /8 $PINGER  0_3_2_3_ 0_3_2_3_ 003_2_33 5_8_7_55 5875 5875 0323 0323 5875 ____ +3&8&7 5&8&7 0&8&7 _ }
		{ $ORGAN      $4(0&5&7 0&5&9) }
	)",
	R"( // GAME OVER
		#SAX !261.62:L250V70A70D40R40w1:L50V90f0.50A98R50w-4:V120D60S0B:V70f3w-4:w-4
		#ORGAN !261.62:L250V50A70D40R40w1:L50V90f0.50A98R50w-4:V120D60S0B:V70f3w-4:w-4
		
		!loop
		{ $SAX q
		    { $ORGAN %16 2&7&9 }
		    297t=9== // 8
		    _2t90'9t9 // 8
		    { $ORGAN %20 0&7&9 } 7==_ // 4
		    0769=7=_ // 8
		    0767979t // 8
		    { $ORGAN %4 2&7&9 }  9==_ // 4
		}
	)",
	R"( // TITLE
		#ORGAN !261.62:L250V65A70D40R40w1:L50V90f0.50A98R50w-4:V120D60S0B:V70f3w-4:w-4
		#TOMTOM !174.61:L2000V100r-8D40S0R35:V90f3r-2D50S0B:V80r2D55S0N:f5r-2:
		#PINGER !415.30:L250V100A98D35S60R30w3:L50V90f0.50A98R50w-4:V120D60S0B:w-4:w-4
		#NOISY !349.23:L250V100A80D30S0R40w1:L50V90f0.70A98R50:V100D30S0B:w0:w0
		#BIGDRUM !174.61:L200V100r-50A90D40R40:V130f7D45S0B:V90D55S0:f5r-2:
		#VIOLIN !698.45:L2500V95A60R40w1:L500A98D50S0R40N:V20f0.25A98D30S0w0:V70f5A90D40:f5r-2w0
		#JBASS !110.00:L250V100A80D40S50R40Q:L500A98D50S0R40N:f0.25A98D30S0w0:f6A90D40:f5r-2w0
		#DBASS !87.31:L250V100A80D40S50R40w2:L500A98D50S0R40N:f0.25A98D30S0w0:f6A90D40:f5r-2w0
		#SEA !587.33:L2500V100A30D20S0R20B:S101:::
		#SOUND !329.63:L2500V100A60D40R30w3:L500000f5A55D40R40:V100f0.25A60D40S0:V60f12A50D40:f5r-2w0
		#SNARE !493.88:L250V90D45S0N:D45S0N:w-4:w-4:w-4
        #BRUM !87.31:L2500V100A60D20S0R30w0:L500000V80f0.02A55D40R40:V120f0.50A60D40:f50A40D40:f5r-2w0
        #BELL !523.25:L250V100A95D70S90R30w0:V90f0.50D38S0:V90f4A60:V70f7:
        #CLEAN !523.25:L250V100A98D45S0R40Q:f0.50D38S0:f4A60:f7:
        #TRUMPET !261.62:L250V100A98D45S90R40w4:f0.50D38S0:f4A60:V55f5A60:f10
		
		!tempo:141
		
		#SPACEFX { !349.23:L5000V100A60D20S0R20:L500000V80f2r3A55D40R40:V120f0.50A60D40:f50A40D40:f5r-2w0 * }
		
		$SPACEFX
		
		!loop
		{ $BRUM -- $8(0 3 2 1) }
		
		#BACK 0&4&7 0&3&7 2&5&9 4&1&8

		{ $ORGAN ____ $BACK $SPACEFX $BACK $BACK $SPACEFX $BACK $BACK $SPACEFX $BACK $BACK  $SPACEFX $BACK $BACK }
		{ $SNARE ____ ____ ____ w $192(*) }
		{ $JBASS ____ ____ ____ ____ w -- $32(0'=00) }
		{ $BIGDRUM $5(____) q $16(*) $32(*)}

$NOISY [!volume:0.8  $6(____) - $32(y 0730' @1)] !cut 

$BELL
        {[!volume:0.5 n %3 $SEA *] }
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM ____ q $16(*) $32(*)}

[ w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e !cut ]

[ @7
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM ____ q $16(*) $32(*)}

[w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e ] ] !cut

		{ !87.31:L2500V100A60D20S0R30w0:L500000V80f0.02A55D40R40:V120f0.50A60D40:f50A40D40:f5r-2w0
		   -- $7(0321) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(0'=00) }
		{ $BIGDRUM q $16(*) $32(*)}

$PINGER
[ - w0'7'm0''.qt'.9'.w7'5'q7'.w2'7'2'7'e'{!volume:0.90 $VIOLIN +++++ x$6(e72-)} 7'======= ]
!cut

$TRUMPET
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM q $16(*) $32(*)}

[ - w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e !cut ]

[ @7
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM q $16(*) $32(*)}

[- w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e ] ] !cut

		{ !87.31:L2500V100A60D20S0R30w0:L500000V80f0.02A55D40R40:V120f0.50A60D40:f50A40D40:f5r-2w0
		   -- $7(0321) }
		{ $SNARE w $160(*) }
		{ $DBASS w -- $32(0'=00) }
		{ $BIGDRUM q $16(*) $32(*)}

$BELL
[ - w0'7'm0''.qt'.9'.w7'5'q7'.w2'7'2'7'e'7'======= ] !cut

$BELL
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM ____ q $16(*) $32(*)}

[ w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e !cut ]

[ @7
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM ____ q $16(*) $32(*)}

[w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e ] ] !cut

		{ !87.31:L2500V100A60D20S0R30w0:L500000V80f0.02A55D40R40:V120f0.50A60D40:f50A40D40:f5r-2w0
		   -- $7(0321) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(0'=00) }
		{ $BIGDRUM q $16(*) $32(*)}

$NOISY
[ - w0'7'm0''.qt'.9'.w7'5'q7'.w2'7'2'7'e'{!volume:0.90 $VIOLIN +++++ x$6(e72-)} 7'======= ]
!cut

$PINGER
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM q $16(*) $32(*)}

[ w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e !cut ]

[ @7
		{ $BRUM -- $8(5432) }
		{ $SNARE w $192(*) }
		{ $DBASS w -- $32(2'=95) }
		{ $BIGDRUM q $16(*) $32(*)}

[w25m9.q7.5.4w259====0'qe.7weq9w25m9w29q7.5.4w25m0'.q2'.0'.e ] ] !cut

		{ !87.31:L2500V100A60D20S0R30w0:L500000V80f0.02A55D40R40:V120f0.50A60D40:f50A40D40:f5r-2w0
		   -- $7(0321) }
		{ $SNARE w $160(*) }
		{ $DBASS w -- $32(0'=00) }
		{ $BIGDRUM q $16(*) $32(*)}

$BELL
[ - w0'7'm0''.qt'.9'.w7'5'q7'.w2'7'2'7'e'{$PINGER !volume:0.85 +++ x__$4(e72-)}7'======= ]

n {$SEA * ___ * ___ * }
	)"
};

int current_music;

int CurrentMusic()
{
	return IsPlayingSequence() ? current_music : TUNE_NONE;
}

void StartMusic(int m)
{
	static SoundSequence score[TUNE_COUNT];
	ONCELOCK {
		for(int i = 0; i < TUNE_COUNT; i++)
			score[i] = ParseQSF(score_qsf[i]);
	}
	if(m > CurrentMusic()) {
		StopMusic();
		PlaySequence(score[current_music = clamp(m, 0, TUNE_COUNT - 1)]);
	}
}

void StopMusic()
{
	StopSequencer();
}
