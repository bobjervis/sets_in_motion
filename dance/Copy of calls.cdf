:a=
:x=by
:leads=leaders
:two=2
:four=4
:eight=8
:half=1/2
:quarter=1/4
:all=everyone
:outsides=ends
-- None
C1271796717
ANYONE back away
		>$back_out($1, $0)
-- None
C1271796717
face in
		>$face(in, $center)
-- None
C1271796717
face out
		>$face(out, $center)
-- None
C1271796717
M1273534366
quarter R_L
face R_L
		>$face($1, $self)
-- None
C1271796717
ANYONE move in
	*squared_set
		>$activate($1, move_ in)
|
	*infacing_ring
		>$move_in($1)
		>$form_set()
		>$activate($1, move_ in)
-- None
C1271796717
move_ in
	*opposite_homes
		>$forward(1)
-- None
C1271796717
left ANYTHING
		>$mirror($1)
-- None
C1271796717
reverse flutterwheel
		>left flutterwheel
-- None
C1271796717
.anyone_
ANYONE ANYTHING
		>$if($can_start($activate($1, $2)), $activate($1, $2), $if($can_start($1 move_in_and $2), $1 move_in_and $2, $activate($1, $2)))
-- None
C1271796717
FRACTION ANYTHING
		>$fractionalize($1, $2)
-- None
C1271796717
ANYTHING FRACTION
		>$2 $1
-- None
C1271796717
ANYTHING twice
		>$1 2 times
-- None
C1271796717
ANYTHING INTEGER times
	+ $2
		>$1
-- None
C1271796717
._and_
ANYTHING and ANYCALL
		>$1
		>$activate($last_active, $2)
-- None
C1271796717
._while_
ANYTHING while ANYCALL
		>$start_together($1, $2)
-- None
C1271796717
check a ring
	*squared_set
		>$form_ring()
|
	*ring
		>$nothing()
-- Basic
C1271796717
circle R_L
	*squared_set
		>$form_ring()
		>$circle($1)
|
	*infacing_ring
		>$circle($1)
-- Basic
C1271796717
circle R_L FRACTION
	*squared_set
		>$form_ring()
		>$circle_fraction($1, $2)
|
	*infacing_ring
		>$circle_fraction($1, $2)
|
	*facing_couples
		>$arc($box_center, $1, $2)
-- Basic
C1271796717
circle R_L home
	*squared_set
		>$form_ring()
		>$circle_home($1)
		>$form_set()
|
	*infacing_ring
		>$circle_home($1)
		>$form_set()
-- Basic
C1271796717
forward and back
	*squared_set
		>$forward_and_back()
|
	*infacing_ring
		>$forward_and_back()
|
	*facing_lines
		>$forward_and_back()
-- Basic
C1271796717
dosado
	*facing
		>$forward_veer(1/2, -1/2)
	+
		>$forward_veer(1/2, 1/2)
	+
		>$forward_veer(-1/2, 1/2)
	+
		>$forward_veer(-1/2, -1/2)
-- Basic
C1271796717
dosado to a wave
		>dosado
		>touch
-- Basic
C1271796717
M1273615865
swing your P_C
		>$face($original_partner, $self)
		>$arc($nose, left, 4/4)
		>boys $forward_veer(1/2, -1/2) while girls $forward_veer_face(1/2, -1/2, -1/2)
		>$form_promenade(forward)
		>as couples $face(promenade, $self)
-- Basic
C1271796717
M1273873253
promenade FRACTION
	*promenade
		>$arc($center, forward, $1)
		>as couples face in
|
		>as couples face right
		>$form_promenade(forward)
		>$arc($center, forward, $1)
		>as couples face in
-- Basic
C1271796717
M1273303262
promenade keep walking
promenade don't stop don't slow down
	*squared_set
		>as couples face right
		>$form_promenade(forward)
		>$arc($center, forward, 1/4)
-- Basic
C1271796717
M1272521584
promenade home
	*squared_set
		>as couples face right
		>promenade home
|
	*promenade
		>$check_sequence(forward)
		>$arc($center, forward, 1/4)
		>$arc($center, forward, $until_home)
		>couples_face_in
|
	*r_l_grand_circle
		>boys $forward_veer(1/2, -1/2) while girls $forward_veer_face(1/2, -1/2, 1/2)
		>$form_promenade(forward)
		>promenade home
|
	*lh_star
		>$form_promenade(forward)
		>promenade home
|
		>as couples face right
		>$form_promenade(forward)
		>promenade home
-- Basic
C1271796717
M1273615162
single file promenade
promenade single file
	*infacing_ring
		>face right
|
	*squared_set
		>$form_ring()
		>face right
|
	*rh_column
		>$form_ring()
		>$face($reverse_promenade, $self)
|
	*lh_column
		>$form_ring()
		>$face(promenade, $self)
-- Basic
C1271796717
M1272431170
wrong way promenade home
	*r_l_grand_circle
		>boys $forward_veer(1/2, -1/2) while girls $forward_veer_face(1/2, -1/2, 1/2)
		>$form_promenade(back)
		>wrong way promenade home
|
	*reverse_promenade
		>$check_sequence(back)
		>$arc($center, forward, 1/4)
		>$arc($center, forward, $until_home)
		>left couples_face_in
|
	*squared_set
		>as couples face left
		>$form_promenade(back)
		>wrong way promenade home
|
	*star
		>$form_promenade(back)
		>wrong way promenade home
-- Basic
C1271796717
M1273467974
star promenade
	*star
		>$form_promenade(back)
|
	*lh_star
		>$form_promenade(forward)
-- Basic
C1271796717
M1273621984
allemande left
left allemande
	*r_l_grand_circle
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$forward_veer(1/2, -1/2)
|
		>allemande left your corner
-- Basic
C1271796717
M1273621775
allemande R_L your P_C
R_L allemande your P_C
	*squared_set
		>$form_ring()
		>$face($2, $self)
		>allemande $1
|
	*two_x_four
		>$form_ring()
		>$face($2, $self)
		>allemande $1
|
	*infacing_ring
		>$face($2, $self)
		>allemande $1
|
	*r_l_grand_circle
		>$face($2, $self)
		>allemande $1
-- Basic
C1271796717
turn
arm turn
full turn
		>turn 4/4
-- Basic
C1271796717
M1273622697
turn FRACTION
arm turn FRACTION
right turn FRACTION
	*rh_mini_wave
	+ (4 * $1)
		>$arc($inside_hand, forward, 1/4)
-- Basic
C1271796717
turn your P_C by the R_L FRACTION
arm turn your P_C by the R_L FRACTION
		>$face($1, $self)
		>$2 touch
		>$2 turn $3
-- Basic
C1271796717
M1273722207
right and left grand
grand right and left
	*rh_waves
		>$start_together(ends $start_together(leads $forward_veer_face(1, 1, 1/4), others $veer(1, in)), others leads $forward_veer_face(1,0,-1/4))
		>$form_ring()
	+
		>$pull_by(left)
	+
		>$pull_by(right)
	+
		>$pull_by(left)
|
	*two_x_four
		>$form_ring()
		>$face(partner, $self)
		>$check_sequence(forward)
		>$pull_by(right)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
	+
		>$pull_by(left)
|
	*r_l_grand_circle
		>$face(partner, $self)
		>$check_sequence(forward)
		>$pull_by(right)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
	+
		>$pull_by(left)
|
	*wrong_way_thar
		>$form_ring()
		>$forward_veer(1/2, 1/2)
		>$check_sequence(forward)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
	+
		>$pull_by(left)
-- Basic
C1271796717
weave the ring
	*r_l_grand_circle
		>pass thru
	+
		>left pass thru
	+
		>pass thru
	+
		>left pass thru
-- Basic
C1271796717
wrong way grand
	*two_x_four
		>$form_ring()
		>$face($original_partner, $self)
		>$check_sequence(back)
		>$pull_by(right)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
	+
		>$pull_by(left)
-- Basic
C1271796717
M1273468345
R_L hand star
	*squared_set
		>left as couples face $1
		>$form_thar()
|
	*infacing_ring
		>square your set
		>left as couples face $1
		>$form_thar()
|
	*facing_couples
-- Basic
C1271796717
M1272429798
pass thru
	*facing
		>$forward(1)
|
	*rh_mini_wave
		>$forward_veer(1/2, 1/2)
|
	*lh_mini_wave
		>$forward_veer(1/2, -1/2)
-- Basic
C1271796717
M1271796735
half sashay
	*couple
		>$dancerA $forward_veer(-1/2, 1/2) while $dancerB $forward_veer(1/2, -1/2)
	+
		>$dancerA $forward_veer(1/2, 1/2) while $dancerB $forward_veer(-1/2, -1/2)
-- Basic
C1271796717
M1272406899
rollaway
rollaway with a half sashay
	*couple
		>$start_together($dancerB $arc_face($inside_hand, forward, 1/4, -1/4), $dancerA $forward_veer(0, 1/2))
	+
		>$start_together($dancerB $arc_face($nose, right, 1/4, -1/4), $dancerA $forward_veer(0, 1/2))
-- Basic
C1271796717
M1273303285
ANYONE in ANYONE sashay
ANYONE center ANYONE sashay
	*infacing_ring
		>$1 $forward(1) while $2 $forward_veer(0, -1)
		>$1 $forward(-1) while $2 $forward_veer(0, -1)
-- Basic
C1271796717
backtrack
	*promenade
		>ends $arc($self, right, 1/2)
		>ends $arc($center, forward, 1/4)
		>$form_thar()
-- Basic
C1271796717
M1273611503
separate
		>face out
		>$arc($inside_dancer, forward, 1/4)
-- Basic
C1271796717
M1273537271
ANYONE separate around 1 to a line
	*couples_on_home($1)
		>$1 face out while others $forward(1)
		>$1 run
|
	*lead_centers_of_two_x_four($1)
		>centers $forward(1)
		>$1 separate around 1 to a line
-- Basic
C1271796717
M1273537375
ANYONE separate around 1 and come into the middle
	*couples_on_home($1)
		>$1 face out while others $forward(1)
		>ends $forward(1) and $arc($inside_hand, forward, 1/2)
		>ends $forward(1) while others $forward(-1)
|
	*lead_centers_of_two_x_four($1)
		>$1 run and roll
		>$1 $forward(1) while others $forward(-1)
-- Basic
C1271796717
M1271807804
ANYONE split 2
	*start_tandem_split_two($1)
		>$1 $forward(1) while others $forward(2)
|
	*start_couple_split_two($1)
		>pass thru
-- Basic
C1271796717
M1272408383
courtesy turn
	*couple
		>$dancerA $arc($inside_hand, back, 1/4) while $dancerB $arc($inside_hand, forward, 1/4)
	+
		>$dancerA $arc($inside_hand, back, 1/4) while $dancerB $arc($inside_hand, forward, 1/4)
|
	*rh_line
		>cast off 1/4 and courtesy turn
|
	*r_l_grand_circle
		>boys $forward_veer(1/2, -1/2) while girls $forward_veer_face(1/2, -1/2, 1/2)
		>boys $arc($right_hand, back, 1/4) while others $arc($left_hand, forward, 1/4)
-- Basic
C1271796717
M1273112241
4 ladies chain
	*squared_set
		>$start_together(boys face left, girls $forward_veer(3/2, -1) and $arc($center, forward, 1/4) and $forward(1/2))
		>girls $arc($left_dancer, forward, 3/4) while boys $arc($self, left, 3/4)
|
	*infacing_ring
		>square your set
		>4 ladies chain
-- Basic
C1271796717
M1273469933
ANYONE chain
	*facing_belles($1)
		>as couples face left
		>centers trade
		>cast off 3/4
-- Basic
C1271796717
chain down the line
	*rh_line
		>centers trade
	+
		>courtesy turn
|
	*lh_wave
		>centers trade while ends u-turn back
	+
		>courtesy turn
-- Basic
C1271796717
M1272407982
do paso
	*r_l_grand_circle
		>$face(partner, $self)
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$forward_veer(1, -1)
		>$arc($right_hand, forward, 1/2)
		>$forward_veer(1/2, 1/2)
		>courtesy turn
-- Basic
C1271796717
veer right
	*facing_couples
		>$forward_veer(1/2, 1)
-- Basic
C1271796717
veer left
		>left veer right
-- Basic
C1271796717
lead right
	*facing_couples
		>beaus $forward_veer_face(1, 1, 1/4) while belles face right
-- Basic
C1271796717
lead left
		>left lead right
-- Basic
C1271796717
bend the line
	*rh_line
	*lh_line
	*one_faced_line
		>as couples quarter in
-- Basic
C1271796717
M1273303304
line of 8 bend the line
bend the big line
work 4 by 4 and bend the line
	*rh_four_x_four_line
	*lh_four_x_four_line
		>as couples as couples face in
-- Basic
C1271796717
M1272407339
couples circulate
		>as couples box_ circulate
-- Basic
C1271796717
M1273645959
circulate
single file circulate
	*lines
		>ends $start_together($any_who_can(leads $arc($inside_hand, forward, 1/4) and $forward(2) and $arc($last_hand, forward, 1/4)), $any_who_can(others $forward(1))) while others box_ circulate
|
	*column
		>$any_who_can(leaders ends $arc($inside_hand, forward, 1/2)) while others $forward(1)
-- Basic
C1271796717
split circulate
	*two_x_four
		>box_ circulate
-- Basic
C1271796717
ANYONE box circulate
		>$activate($1, box_ circulate)
-- None
C1271796717
box_ circulate
	*box
		>$any_who_can(trailers $forward(1)) while $any_who_can(others $arc($inside_hand, forward, 1/2))
-- Basic
C1271796717
right and left thru
	*facing_couples
		>$pull_by(right)
	+
		>courtesy turn
-- Basic
C1271796717
grand square
sides face grand square
	*squared_set
		>heads $forward(1) while sides face in and $forward(-1)
		>face in
	+
		>heads $forward(-1) while sides $forward(1)
		>face in
	+
		>heads $forward(-1) while sides $forward(1)
		>face in
	+
		>heads $forward(1) while sides $forward(-1)
	+
		>heads $forward(-1) while sides $forward(1)
		>face in
	+
		>heads $forward(1) while sides $forward(-1)
		>face in
	+
		>heads $forward(1) while sides $forward(-1)
		>face in
	+
		>heads $forward(-1) while sides $forward(1) and face in
-- Basic
C1271796717
star thru
	*facing
		>pass thru
		>boys face right while girls face left
-- Basic
C1271796717
circle to a line
	*facing_couples
		>circle left 1/2
		>$if($closer_to_center($dancerB, $dancerC), $dancerC $forward_veer(1/2, -1) while $dancerD $forward_veer(1/2, -1) while $dancerB $forward_veer_face(1/2, 0, 1/2) while $dancerA $forward_veer_face(1/2, -2, 1/2), $dancerB $forward_veer(1/2, -1) while $dancerA $forward_veer(1/2, -1) while $dancerC $forward_veer_face(1/2, 0, 1/2) while $dancerD $forward_veer_face(1/2, -2, 1/2))
-- Basic
C1271796717
walk around the corner
walk around your corner
	*squared_set
		>$form_ring()
		>boys face left while girls face right
		>$forward_veer(1/2, -1/2)
		>$arc($right_hand, forward, 1/2)
		>$forward_veer(1/2, 1/2)
-- Basic
C1271796717
see saw
	*r_l_grand_circle
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$forward_veer(1/2, -1/2)
-- Basic
C1271796717
square thru
		>square thru 4
-- Basic
C1271796717
square thru INTEGER
	*facing_couples
		>$pull_by(right)
	+ $1 - 1
		>$face(in, $box_center)
		>$pull_by(right)
-- Basic
C1271796717
M1272495447
california twirl
	*normal_couple
		>trade
-- Basic
C1271796717
M1272409221
dive thru
	*eight_chain_thru
		>ends split two
		>ends california twirl
-- Basic
C1271796717
M1274136511
ANYONE dive thru
	*facing_couples
		>pass thru
		>$1 $nothing() while others california twirl 
-- Basic
C1271796717
wheel around
	*couple
		>$dancerA $arc($inside_hand, back, 1/4) while $dancerB $arc($inside_hand, forward, 1/4)
	+
		>$dancerA $arc($inside_hand, back, 1/4) while $dancerB $arc($inside_hand, forward, 1/4)
-- Basic
C1271796717
M1272430118
allemande left to an allemande thar
	*squared_set
		>$form_ring()
		>boys face left while girls face right
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$forward_veer(1/2, -1/2)
		>$pull_by(right)
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$form_thar()
|
	*r_l_grand_circle
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$forward_veer(1/2, -1/2)
		>$pull_by(right)
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 1/2)
		>$form_thar()
-- Basic
C1271796717
M1272521415
shoot the star to another thar
	*thar
		>shoot the star
		>$form_ring()
		>$forward_veer(1/2, -1/2)
		>$pull_by(right)
		>$forward_veer(1/2, 1/2)
		>$form_thar()
		>left turn 1/2
|
	*wrong_way_thar
		>shoot the star
		>$form_ring()
		>$forward_veer(1/2, 1/2)
		>$pull_by(left)
		>$forward_veer(1/2, -1/2)
		>$form_thar()
		>turn 1/2
-- Basic
C1271796717
M1272521728
make an allemande thar
		>$form_thar()
-- Basic
C1271796717
M1272521182
make a wrong way thar
		>$form_thar()
-- Basic
C1271796717
M1272430672
shoot the star
	*thar
		>left turn 1/2
|
	*wrong_way_thar
		>turn 1/2
-- Basic
C1271796717
M1272430911
shoot the star full around
	*thar
		>left turn
|
	*wrong_way_thar
		>turn
-- Basic
C1271796717
M1271797494
slip the clutch
	*thar
		>$rotate(-1/8, ends $arc($center, forward, 1/4))
|
	*wrong_way_thar
		>$rotate(1/8, ends $arc($center, forward, 1/4))
-- Basic
C1271796717
.box_the_gnat
box the gnat
	*facing
		>$forward_veer(1/2, -1/2)
		>boys $arc_face($inside_hand, forward, 1/4, 1/4) while girls $arc_face($inside_hand, forward, 1/4, -3/4)
-- Basic
C1271796717
M1272480803
trade
	*adjacent_pair
	^Preferred
		>$arc($inside_hand, forward, 1/2)
|
	*end_pair
		>$arc($inside_hand, forward, 1/4)
		>$forward(2)
		>$arc($last_hand, forward, 1/4)
|
	*cross_pair
		>$arc($inside_hand, forward, 1/4)
		>$forward(1)
		>$arc($last_hand, forward, 1/4)
|
	*infacing_ring
		>normal_couple trade
|
	*alamo_ring
		>turn 1/2
-- Basic
C1271796717
couples trade
		>as couples trade
-- Basic
C1271796717
partner trade
		>trade
-- Basic
C1271796717
step to a wave
		>touch
-- Basic
C1271796717
M1272522973
allemande left in the alamo style
	*squared_set
		>$form_ring()
		>$face(corner, $self)
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 3/4)
|
	*infacing_ring
		>$face(corner, $self)
		>$forward_veer(1/2, 1/2)
		>$arc($left_hand, forward, 3/4)
-- Basic
C1271796717
M1273698151
swing thru
	*facing_couples
		>beaus $forward_veer(1/2, -1) while belles $forward(1/2)
		>those who can turn 1/2
	+
		>those who can left turn 1/2
|
	*line
	*ring
		>those who can turn 1/2
	+
		>those who can left turn 1/2
-- Basic
C1271796717
ANYONE run
		>$run($1)
-- Basic
C1271796717
M1272583970
ANYONE cross run
	*centers_of_line($1)
		>$1 $arc($inside_dancer, forward, 1/2) while others $veer(1, in) 
|
	*ends_of_line($1)
		>$1 $arc($inside_dancer, forward, 1/2) while others $veer(1, out) 
-- Basic
C1271796717
pass the ocean
	*facing_couples
		>pass thru
	+
		>face in
	+
		>touch
|
	*rh_wave
	!Plus
		>pass thru
	+
		>face in
	+
		>touch
|
	*lh_wave
	!Plus
		>pass thru
	+
		>face in
	+
		>touch
-- Basic
C1271796717
extend
	*qtr_tag
		>ends $start_together(beaus $forward_veer(1/2, -1), belles $forward(1/2)) while centers $forward(1/2)
|
	*three_qtr_tag
	!Plus
		>ends $forward(1/2) while $dancerC $forward_veer(1/2, -1) while $dancerF $forward_veer(1/2, -1) while $dancerD $forward(1/2) while $dancerE $forward(1/2)
|
	*rh_waves
	!Plus
		>ends $forward_veer(1/2, 1) while centers $forward(1/2)
|
	*lh_waves
	!Plus
		>left extend
|
	*beg_dbl_pass_thru
	!Plus
|
	*left_qtr_tag
		>left extend
-- Basic
C1271796717
wheel and deal
	*one_faced_line
		>as couples single wheel
|
	*rh_line
		>as couples single wheel
|
	*lh_line
		>as couples single wheel
-- Basic
C1271796717
double pass thru
	*beg_dbl_pass_thru
		>$forward(2)
-- Basic
C1271796717
M1273460833
first couple go R_L, next couple go R_L
	*tandem_couples
		>as couples first dancer go $1 next dancer go $2
-- Basic
C1271796717
zoom
	*box
		>$start_together(leaders $arc($outside_hand, forward, 1/2) and $forward(1) and $arc($last_hand, forward, 1/2), others $forward(1))
|
	*tandem
		>$start_together(leaders $arc($outside_hand, forward, 1/2) and $forward(1) and $arc($last_hand, forward, 1/2), others $forward(1))
-- Basic
C1271796717
flutterwheel
	*facing_couples
		>$forward_veer(1/2, -1)
		>centers turn 1/2
		>wheel and deal
-- Basic
C1271796717
sweep a 1/4
	*facing_couples
		>$has_lateral_flow($as_you_are)
		>$if($has_lateral_flow(left), circle left 1/4, circle right 1/4)
-- Basic
C1271796717
trade by
	*trade_by
		>ends trade while centers pass thru
-- Basic
C1271796717
M1273623218
touch
	*facing
		>$forward_veer(1/2, -1/2)
-- Basic
C1271796717
touch FRACTION
	*facing
		>$forward_veer(1/2, -1/2)
		>$arc($inside_hand, forward, 1/4)
	+ (4 * $1) - 1
		>$arc($inside_hand, forward, 1/4)
-- Basic
C1271796717
ferris wheel
		>stretch wheel and deal
-- Mainstream
C1271796717
slide thru
	*facing
		>pass thru
		>$any_who_can(boys face right) while $any_who_can(girls face left)
-- Mainstream
C1271796717
M1274137362
cast off FRACTION
	*one_faced_line
		>as couples $arc($self, out, $1)
|
	*rh_line
	+ (4 * $1)
		>as couples quarter left
|
	*lh_line
	+ (4 * $1)
		>as couples quarter right
|
	*rh_mini_wave
	*lh_mini_wave
		>$arc($inside_hand, forward, $1)
-- Advanced-1
C1271796717
as couples ANYTHING
		>$reduce(couple, couple, $1)
-- Advanced-1
C1271796717
quarter in
	*box
		>$face(in, $box_center)
|
		>$face(in, $box_center)
-- Advanced-1
C1271796717
M1274137255
quarter out
	*box
		>$face(out, $box_center)
|
		>$face(out, $self)
-- Challenge-1
C1271796717
in tandem ANYTHING
		>$reduce(tandem, tandem, $1)
-- Challenge-1
C1271796717
siamese ANYTHING
		>$reduce(couple, tandem, $1)
-- None
C1271796717
those who can ANYTHING
		>$those_who_can($1)
-- Basic
C1271796717
M1273460842
first dancer go R_L, next dancer go R_L
	*tandem
		>$if($1 != $2, leads $forward_peel(-1/2,$1) while trailers $forward_peel(1/2,$2), leads $arc($hand, $1, 1/4) while trailers $forward(1) and $arc($self, $2, 1/4))
-- None
C1271796717
square your set
	*infacing_ring
		>$form_set()
-- Advanced-2
C1271796717
single wheel
	*rh_mini_wave
	*lh_mini_wave
		>$arc($inside_hand, forward, 1/4)
		>roll
|
	*couple
		>quarter in
		>$forward_veer(1/2, -1/2)
		>$dancerA face right while $dancerB face left
-- Plus
C1271796717
roll
		>$roll()
-- Basic
C1271796717
M1273622077
allemande right
right allemande
	*r_l_grand_circle
		>$forward_veer(1/2, -1/2)
		>$arc($right_hand, forward, 1/2)
		>$forward_veer(1/2, 1/2)
|
		>allemande right your partner
-- Basic
C1271796717
u-turn back
		>$arc($self, in, 1/2)
-- Basic
C1271796717
single circle R_L FRACTION
	*facing
		>$arc($nose, $1, $2)
-- Basic
C1271796717
M1273303371
grand square INTEGER steps
		>($1 / 32) grand square
-- Basic
C1271796717
ANYONE run R_L
		>$run_to($1, $2)
-- None
C1271796717
M1273705033
ANYONE move_in_and ANYTHING
	*squared_set
		>$1 move in and $2 and $if($definition($2, box_the_gnat), $nothing(), $back_out($1, $0))
|
	*infacing_ring
		>$move_in($1)
		>$form_set()
		>$1 move_in_and $2
-- None
C1271796717
couples_face_in
	*couple
		>$dancerA $arc($inside_hand, back, 1/4) while $dancerB $arc($inside_hand, forward, 1/4)
-- None
C1272480729
normal_couple trade
	*normal_couple
		>trade
-- Basic
C1272565947
M1272566020
single file promenade FRACTION
	*infacing_ring
		>single file promenade
		>$forward($1 * 8)
|
	*squared_set
		>single file promenade
		>$forward($1 * 8)
-- Basic
C1272566194
M1272566311
single file promenade home
	*squared_set
		>single file promenade
		>$check_sequence(forward)
		>$arc($center, forward, $until_home)
		>face in
		>square your set
|
	*infacing_ring
		>single file promenade
		>$check_sequence(forward)
		>$arc($center, forward, $until_home)
		>face in
		>square your set
-- Challenge-1
C1273042327
M1273042351
stretch ANYTHING
		>$stretch($1)
-- Basic
C1273108709
M1273646129
ANYONE chain 3/4
	*squared_set_belles($1)
|
	*infacing_ring_belles($1)
-- Basic
C1273108826
M1273184391
4 ladies chain 3/4
	*squared_set
		>$start_together(boys face left, girls $forward_veer(3/2, -1) and $arc($center, forward, 1/2) and $forward(1/2))
		>girls $arc($left_dancer, forward, 3/4) while boys $arc($self, left, 3/4)
|
	*infacing_ring
		>square your set
		>4 ladies chain 3/4
-- Basic
C1273537144
M1273537478
ANYONE separate, around 2 to a line
	*couples_on_home($1)
		>$1 face out while others $forward(1)
		>ends trade
|
	*lead_centers_of_two_x_four($1)
		>$1 $forward(1)
		>$1 separate around 2 to a line
-- Advanced-1
C1273610069
M1273611495
ends bend
	*ends_of_line
		>ends $arc($inside_dancer, forward, 1/4)
-- Basic
C1273613191
M1273613370
wrong way promenade FRACTION
	*star
		>$form_promenade(back)
		>$arc($center, forward, $1)
		>as couples face in
|
		>as couples face left
		>$form_promenade(back)
		>$arc($center, forward, $1)
		>as couples face in
-- Mainstream
C1274135872
M1274157385
cloverleaf
	*comp_dbl_pass_thru
		>$reduce(tandem, tandem, $forward(1/2) and $arc($outside_hand, forward, 3/4))
|
	*lead_centers_trailing_ends
		>centers run and roll
-- Mainstream
C1274157403
M1274157489
ANYONE cloverleaf
	*lead_centers_trailing_ends($1)
		>centers run and roll
|
	*lead_centers_of_two_x_four($1)
		>centers $arc($outside_dancer, forward, 1/2) and roll
-- Mainstream
C1274157568
M1274157652
turn thru
	*facing
		>$forward_veer(1/2, -1/2)
		>$arc($inside_hand, forward, 1/2)
		>$forward_veer(1/2, 1/2)
-- Mainstream
C1274167721
M1274206847
8 chain thru
		>$pull_by(right)
	+
		>ends courtesy turn while centers $pull_by(left)
	+
		>$pull_by(right)
	+
		>ends courtesy turn while centers $pull_by(left)
	+
		>$pull_by(right)
	+
		>ends courtesy turn while centers $pull_by(left)
	+
		>$pull_by(right)
	+
		>ends courtesy turn while centers $pull_by(left)
-- Mainstream
C1274167821
M1274206697
8 chain INTEGER
	*eight_chain_thru
		>$fractionalize($1 / 8, eight chain thru)
-- Mainstream
C1274206927
M1274206963
pass to the center
	*eight_chain_thru
		>pass thru
		>ends trade
-- Mainstream
C1274207089
M1274207102
couples hinge
		>as couples hinge
-- Mainstream
C1274207110
M1274220478
hinge
hinge a 1/4
partner hinge
	*rh_mini_wave
	*lh_mini_wave
		>$arc($inside_hand, forward, 1/4)
|
	*couple
		>$dancerA $arc($inside_hand, forward, 1/4) while others $arc($tail, left, 1/4)
-- Mainstream
C1274207594
M1274207912
centers in
	*comp_dbl_pass_thru
		>ends $veer(1, out) and $forward(-1)
|
	*eight_chain_thru
		>ends $veer(1, out) and $forward(1)
-- Challenge-1
C1274207648
M1274207666
press ahead
	*dancer
		>$forward(1)
-- Mainstream
C1274213227
M1274213478
spin the top
	*rh_wave
	*lh_wave
		>trade
	+
		>fan the top
-- Plus
C1274213490
M1274213583
fan the top
	*rh_wave
	*lh_wave
		>centers cast off 3/4 while others $arc($box_center, forward, 1/4)
-- Mainstream
C1274213732
M1274213932
walk and dodge
	*box
		>$start_together($any_who_can(trailers $forward(1)), $any_who_can(others $veer(1, in)))
-- Mainstream
C1274213990
M1274214035
ANYONE walk, ANYONE dodge
	*facing_couples
		>$1 $forward(1) while $2 $veer(1, in)
-- Mainstream
C1274471953
M1274473044
ANYONE fold
	*dancer_and_adjacent($1)
		>$1 $forward(1)
		>$1 $arc($inside_hand, forward, 1/2)
%%
=squared_set
*C1274473265
. . av . av
. . . . .
a> . . . . . a<
. . . . . . .
a> . . . . . a<
. . . . .
. . a^ . a^
=squared_set_belles
*C1274473265
. . dv . nv
. . . . .
n> . . . . . n<
. . . . . . .
n> . . . . . n<
. . . . .
. . n^ . d^
=opposite_homes
*C1274473265
a> . . . . . a<
. . . . . . .
a> . . . . . a<
=couples_on_home
*C1274473265
. . d| . d|
. . . . .
n? . . . . . n?
. . . . . . .
n? . . . . . n?
. . . . .
. . d| . d|
=lines
*C1274473265
a| . a| . a| . a|
. . . . . . .
a| . a| . a| . a|
=rh_waves
*C1274473265
a^ . av . a^ . av
. . . . . . .
a^ . av . a^ . av
=column
*C1274473265
a- . a- . a- . a-
. . . . . . .
a- . a- . a- . a-
=facing_lines
*C1274473265
av . av . av . av
. . . . . . .
a^ . a^ . a^ . a^
=beg_dbl_pass_thru
*C1274473265
a> . a> . a< . a<
. . . . . . .
a> . a> . a< . a<
=comp_dbl_pass_thru
*C1274473265
a< . a< . a> . a>
. . . . . . .
a< . a< . a> . a>
=diamond
*C1274473265
. e- .
. . .
c| . c|
. . .
. e- .
=box
*C1274473265
a? . a?
. . .
a? . a?
=rh_box
*C1274473265
a^ . av
. . .
a^ . av
=lh_box
*C1274473265
av . a^
. . .
av . a^
=facing_belles
*C1274473265
dv . nv
.
n^ . d^
=facing_couples
*C1274473265
av . av
. . .
a^ . a^
=tandem_couples
*C1274473265
a^ . a^
. . .
a^ . a^
=one_x_four
*C1274473265
e? . c? . c? . e?
=two_x_four
*C1274473265
e? . c? . c? . e?
.
e? . c? . c? . e?
=rh_wave
*C1274473265
a^ . av . a^ . av
=lh_wave
*C1274473265
av . a^ . av . a^
=rh_line
*C1274473265
a^ . a^ . av . av
=lh_line
*C1274473265
av . av . a^ . a^
=one_faced_line
*C1274473265
a^ . a^ . a^ . a^
=line
*C1274473265
a| . a| . a| . a|
=inverted_line
*C1274473265
av . a^ . a^ . av
=trade_by
*C1274473265
a^ . a^
.
av . av
.
a^ . a^
.
av . av
=one_x_eight
*C1274473265
E? . e? . c? . C? . C? . c? . e? . E?
=tidal
*C1274473265
a| . a| . a| . a| . a| . a| . a| . a|
=rh_four_x_four_line
*C1274473265
a^ . a^ . a^ . a^ . av . av . av . av
=lh_four_x_four_line
*C1274473265
av . av . av . av . a^ . a^ . a^ . a^
=two_four_two_lines
*C1274473265
. . e| . e|
. . . . .
c| . C| . C| . c|
. . . . .
. . e| . e|
=twin_diamonds
*C1274473265
. e- . . . e-
. . . . . .
c| . C| . C| . c|
. . . . . .
. e- . . . e-
=pt_to_pt_diamonds
*C1274473265
. . c- . . . . . c-
E| . . . C| . C| . . . E|
. . c- . . . . . c-
=qtr_tag
*C1274473265
. . av . av
. . . . .
a^ . av . a^ . av
. . . . .
. . a^ . a^
=left_qtr_tag
*C1274473265
. . av . av
. . . . .
av . a^ . av . a^
. . . . .
. . a^ . a^
=three_qtr_tag
*C1274473265
. . a^ . a^
. . . . .
a^ . av . a^ . av
. . . . .
. . av . av
=left_three_qtr_tag
*C1274473265
. . a^ . a^
. . . . .
av . a^ . av . a^
. . . . .
. . av . av
=prom_star
*C1274473265
. . . . e?
. . . . .
. . . . c?
. . . . .
e? . c? . . . c? . e?
. . . . .
. . . . c?
. . . . .
. . . . e?
=promenade
*C1274473265
. . . . a<
. . . . .
. . . . a<
. . . . .
av . av . . . a^ . a^
. . . . .
. . . . a>
. . . . .
. . . . a>
=reverse_promenade
*C1274473265
. . . . a>
. . . . .
. . . . a>
. . . . .
a^ . a^ . . . av . av
. . . . .
. . . . a<
. . . . .
. . . . a<
=thar_star
*C1274473265
. . . e-
. . . .
. . . c-
e| . c| . c| . e|
. . . c-
. . . .
. . . e-
=thar
*C1274473265
. . . a<
. . . .
. . . a>
av . a^ . av . a^
. . . a<
. . . .
. . . a>
=wrong_way_thar
*C1274473265
. . . a>
. . . .
. . . a<
a^ . av . a^ . av
. . . a>
. . . .
. . . a<
=star
*C1274473265
. . . a>
. . . .
. . . a>
a^ . a^ . av . av
. . . a<
. . . .
. . . a<
=lh_star
*C1274473265
. . . a<
. . . .
. . . a<
av . av . a^ . a^
. . . a>
. . . .
. . . a>
=start_tandem_split_two
*C1274473265
. . . . n-
d> . d> . .
. . . . n-
=start_couple_split_two
*C1274473265
d> . n-
.
d> . n-
=lead_centers_of_two_x_four
*C1274473265
n? . d^ . d^ . n?
.
n? . dv . dv . n?
=lead_centers_trailing_ends
*C1274473265
n> . d^ . d^ . n<
.
n> . dv . dv . n<
=centers_of_line
*C1274473265
n? . d| . d| . n?
=ends_of_line
*C1274473265
d| . n? . n? . d|
=eight_chain_thru
*C1274473265
a> . a< . a> . a<
.
a> . a< . a> . a<
=rh_column
*C1274473265
a> . a> . a> . a>
.
a< . a< . a< . a<
=lh_column
*C1274473265
a< . a< . a< . a<
.
a> . a> . a> . a>
=adjacent_pair
*C1274473265
a| . a|
=end_pair
*C1274473265
a| . . . . . a|
=end_pairs
*C1274473265
a| . . . . . a|
.
a| . . . . . a|
=cross_pair
*C1274473265
a| . . . a|
=couple @grid
*C1274473265
a^ . a^
=normal_couple
*C1274473265
b^ . g^
=rh_mini_wave
*C1274473265
a^ . av
=lh_mini_wave
*C1274473265
av . a^
=twosome
*C1274473265
a- . a-
=tandem
*C1274473265
a> . a>
=facing
*C1274473265
a> . a<
=back_to_back
*C1274473265
a< . a>
=dancer
*C1274473265
a^
=infacing_ring @ring
*C1274473265
av . av . av . av . av . av . av . av . \
=alamo_ring @ring
*C1274473265
a^ . av . a^ . av . a^ . av . a^ . av . \
=ring @ring
*C1274473265
a| . a| . a| . a| . a| . a| . a| . a| . \
=r_l_grand_circle @ring
*C1274473265
a> . a< . a> . a< . a> . a< . a> . a< . \
=dancer_and_adjacent
*C1274473265
*M1274473280
