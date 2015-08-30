:a=
:x=by
:leads=leaders
:two=2
:four=4
:eight=8
:half=1/2
:quarter=1/4
:everyone=all
:outsides=ends
-- None
C1271796717
M1293672850
back away
	*centers_facing
		>$forward(-1)
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
M1281296321
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
M1293838388
.transparent_
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
M1275108169
.transparent_
ANYTHING twice
		>$1 2 times
-- None
C1271796717
ANYTHING INTEGER times
	+ $2
		>$1
-- None
C1271796717
M1275683395
.transparent_
ANYTHING and ANYCALL
		>$1
	+
		>$activate($last_active, $2)
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
M1277421601
circle R_L
	*squared_set
		>$form_ring()
		>$circle($1)
|
	*infacing_ring
		>$circle($1)
|
	*sausage_line
		>$form_ring()
		>circle $1
|
	*facing_lines
		>$form_ring()
		>circle $1
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
M1298256439
circle R_L home
	*squared_set
	*facing_lines
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
M1294100857
swing your partner
	*normal_couple
		>$face(in, $box_center)
		>$arc($nose, left, 4/4)
		<
			@F boys
			#F $forward_veer(1/2, -1/2)
			@F girls
			#F $forward_veer_face(1/2, -1/2, -1/2)
|
	*r_l_grand_circle
		>$face($original_partner, $self)
		>$arc($nose, left, 4/4)
		<
			@F boys
			#F $forward_veer(1/2, -1/2)
			@F girls
			#F $forward_veer_face(1/2, -1/2, -1/2)
		>$form_promenade(forward)
		>as couples $face(promenade, $self)
-- Basic
C1271796717
M1277855671
promenade FRACTION
	*promenade
		>$arc($center, forward, $1)
		>couples_face_in
|
		>as couples face right
		>$form_promenade(forward)
		>$arc($center, forward, $1)
		>couples_face_in
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
M1295242829
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
		<
			@F boys
			#F $forward_veer(1/2, -1/2)
			@F girls
			#F $forward_veer_face(1/2, -1/2, 1/2)
		>$form_promenade(forward)
		>promenade home
|
	*lh_star
		>$form_promenade(forward)
		>promenade home
|
	*lh_lines
		>as couples box_ circulate 1/2
		>$form_promenade(forward)
		>promenade home
|
	*two_x_four
		>as couples face right
		>$form_promenade(forward)
		>promenade home
|
	*infacing_ring
		>$form_set()
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
M1294097129
wrong way promenade home
	*r_l_grand_circle
		<
			@F boys
			#F $forward_veer(1/2, -1/2)
			@F girls
			#F $forward_veer_face(1/2, -1/2, 1/2)
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
|
	*rh_lines
		>as couples 1/2 box_ circulate
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
M1280182048
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
M1298257405
right and left grand
grand right and left
	*rh_waves
		<
			@F lead ends
			#F $forward_veer(1, 1)
			@F other ends
			#F $veer(1, in)
			@F other leads
			#F $forward(1)
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
		<
			@F boys
			#F $face(promenade, $self)
			@F girls
			#F $face($reverse_promenade, $self)
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
		<
			@F boys
			#F $face(promenade, $self)
			@F girls
			#F $face($reverse_promenade, $self)
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
|
	*line_between_pairs
	*prom_thar
		>$form_thar()
		>right and left grand
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
M1280189393
half sashay
	*couple
		<
			@F $dancerA
			#F $forward_veer(-1/2, 1/2)
			@F $dancerB
			#F $forward_veer(1/2, -1/2)
	+
		<
			@F $dancerA
			#F $forward_veer(1/2, 1/2)
			@F $dancerB
			#F $forward_veer(-1/2, -1/2)
-- Basic
C1271796717
M1294098895
.$dancerA
rollaway
rollaway with a half sashay
	*couple
		<
			@F $dancerB
			#F $arc_face($inside_hand, forward, 1/4, -1/4)
			@F $dancerA
			#F $forward_veer(0, 1/2)
	+
		<
			@F $dancerB
			#F $arc_face($nose, right, 1/4, -1/4)
			@F $dancerA
			#F $forward_veer(0, 1/2)
-- Basic
C1271796717
M1294099810
ANYONE in ANYONE sashay
ANYONE center ANYONE sashay
	*infacing_ring
		<
			@F $1
			#F $forward(1)
			@F $2
			#F $forward_veer(0, -1)
		<
			@F $1
			#F $forward(-1)
			@F $2
			#F $forward_veer(0, -1)
-- Basic
C1271796717
backtrack
	*promenade
		>ends $arc($self, right, 1/2)
		>ends $arc($center, forward, 1/4)
		>$form_thar()
-- Basic
C1271796717
M1301808208
separate
		<
			@F cross_facing in-facing ends
			#T u-turn back
			@F others
			#F face out
		>$arc($inside_dancer, forward, 1/4)
-- Basic
C1271796717
M1294101365
ANYONE separate around 1 to a line
	*couples_on_home($1)
		<
			@F $1
			#F face out
			@F others
			#F $forward(1)
		>$1 run
|
	*lead_centers_of_two_x_four($1)
		>centers $forward(1)
		>$1 separate around 1 to a line
-- Basic
C1271796717
M1294101510
ANYONE separate around 1 and come into the middle
	*couples_on_home($1)
		<
			@F $1
			#F face out
			@F others
			#F $forward(1)
		>ends $forward(1) and $arc($inside_hand, forward, 1/2)
		<
			@F ends
			#F $forward(1)
			@F others
			#F $forward(-1)
|
	*lead_centers_of_two_x_four($1)
		>$1 run and roll
		<
			@F $1
			#F $forward(1)
			@F others
			#F $forward(-1)
-- Basic
C1271796717
M1294099711
ANYONE split 2
	*start_tandem_split_two($1)
		<
			@F $1
			#F $forward(1)
			@F others
			#F $forward(2)
|
	*start_couple_split_two($1)
		>pass thru
-- Basic
C1271796717
M1294100123
courtesy turn
	*couple
		<
			@F $dancerA
			#F $arc($inside_hand, back, 1/4)
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4)
	+
		<
			@F $dancerA
			#F $arc($inside_hand, back, 1/4)
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4)
|
	*rh_line
		>cast off 1/4 and courtesy turn
|
	*r_l_grand_circle
		<
			@F boys
			#F $forward_veer(1/2, -1/2)
			@F girls
			#F $forward_veer_face(1/2, -1/2, 1/2)
		<
			@F boys
			#F $arc($right_hand, back, 1/4)
			@F girls
			#F $arc($left_hand, forward, 1/4)
-- Basic
C1271796717
M1294096662
4 ladies chain
	*squared_set
		<
			@F boys
			#F face left
			@F girls
			#F $forward_veer(3/2, -1) and $arc($center, forward, 1/4) and $forward(1/2)
		<
			@F boys
			#F $arc($self, left, 3/4)
			@F girls
			#F $arc($left_dancer, forward, 3/4)
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
M1294099952
chain down the line
	*rh_line
		>centers trade
	+
		>courtesy turn
|
	*lh_wave
		<
			@F centers
			#F trade
			@F ends
			#F u-turn back
	+
		>courtesy turn
-- Basic
C1271796717
M1279936475
do paso
	*r_l_grand_circle
		>$face($original_partner, $self)
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
M1294099550
lead right
	*facing_couples
		<
			@F beaus
			#F $forward_veer_face(1, 1, 1/4)
			@F belles
			#F face right
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
M1303002571
circulate
single file circulate
	*end_pairs
		<
			@F leads
			#T $arc($line_center, forward, 1/2)
			@F others
			#T $forward(1)
|
	*box
		>all box_ circulate
|
	*two_x_four
		<
			@F lead end $facing_along
			#T $arc($inside_hand, forward, 1/2)
			@F lead center $facing_across
			#T $arc($inside_hand, forward, 1/2)
			@F lead end $facing_across
			#T $arc($line_center, forward, 1/2)
			@F others
			#T $forward(1)
|
	*sausage
		<
			@F very ends
			#F $arc($inside_hand, forward, 1/4)
			@F others
			#F $forward(1/2)
	+
		>1/2 circulate
-- Basic
C1271796717
split circulate
	*two_x_four
		>box_ circulate
-- Basic
C1271796717
M1280182856
box circulate
	*center_box
	*box_between_i_pairs
		>box_ circulate
-- None
C1271796717
M1281242785
box_ circulate
	*box
		<
			@F trailers
			#T $forward(1)
			@F others
			#T $arc($inside_hand, forward, 1/2)
-- Basic
C1271796717
M1294101127
right and left thru
	*facing_couples
		>$pull_by(right)
	+
		>courtesy turn
|
	*rh_wave
		<
			@F centers
			#F $forward(1/2)
			@F others
			#F $forward_veer(1/2, 1)
	+
		>courtesy turn
-- Basic
C1271796717
M1294099359
grand square
sides face grand square
	*squared_set
		<
			@F heads
			#F $forward(1)
			@F sides
			#F face in and $forward(-1)
		>face in
	+
		<
			@F heads
			#F $forward(-1)
			@F sides
			#F $forward(1)
		>face in
	+
		<
			@F heads
			#F $forward(-1)
			@F sides
			#F $forward(1)
		>face in
	+
		<
			@F heads
			#F $forward(1)
			@F sides
			#F $forward(-1)
	+
		<
			@F heads
			#F $forward(-1)
			@F sides
			#F $forward(1)
		>face in
	+
		<
			@F heads
			#F $forward(1)
			@F sides
			#F $forward(-1)
		>face in
	+
		<
			@F heads
			#F $forward(1)
			@F sides
			#F $forward(-1)
		>face in 
	+
		<
			@F heads
			#F $forward(-1)
			@F sides
			#F $forward(1) and face in
-- Basic
C1271796717
M1294097166
star thru
	*boy_facing_girl
		>pass thru
		<
			@F boys
			#F face right
			@F girls
			#F face left
-- None
C1271796717
M1298240180
circle_to_a_line_C
	*facing_couples
		<
			@F $dancerA
			#F $forward_veer_face(1/2, -2, 1/2)
			@F $dancerB
			#F $forward_veer_face(1/2, 0, 1/2)
			@F $dancerC
			#F $forward_veer(1/2, -1)
			@F $dancerD
			#F $forward_veer(1/2, -1)
-- Basic
C1271796717
M1294100400
walk around the corner
walk around your corner
	*squared_set
		>$form_ring()
		>walk around the corner
|
	*ring
		<
			@F boys
			#F face left
			@F girls
			#F face right
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
M1293696561
ANYONE dive thru
	*facing_couples
		>pass thru
		>not $1 california twirl 
-- Basic
C1271796717
M1294099656
wheel around
	*couple
		<
			@F $dancerA
			#F $arc($inside_hand, back, 1/4)
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4)
	+
		<
			@F $dancerA
			#F $arc($inside_hand, back, 1/4)
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4)
-- Basic
C1271796717
M1294099485
allemande left to an allemande thar
	*squared_set
		>$form_ring()
		<
			@F boys
			#F face left
			@F girls
			#F face right
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
M1295203913
slip the clutch
	*thar
		>$rotate(-1/8, ends $arc($center, forward, 1/4))
|
	*wrong_way_thar
		>$rotate(1/8, ends $arc($center, forward, 1/4))
|
	*prom_star
		>$form_thar()
		>slip the clutch
-- Basic
C1271796717
M1280189482
.box_the_gnat
box the gnat
	*boy_facing_girl
		>$forward_veer(1/2, -1/2)
		<
			@F boys
			#F  $arc_face($inside_hand, forward, 1/4, 1/4)
			@F girls
			#F $arc_face($inside_hand, forward, 1/4, -3/4)
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
M1275458805
step to a wave
	*facing_couples
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
M1294096869
swing thru
	*facing_couples
		<
			@F beaus
			#F $forward_veer(1/2, -1)
			@F belles
			#F $forward(1/2)
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
M1294099906
ANYONE cross run
	*centers_of_line($1)
		<
			@F $1
			#F $arc($inside_dancer, forward, 1/2)
			@F others
			#F $veer(1, in) 
|
	*ends_of_line($1)
		<
			@F $1
			#F $arc($inside_dancer, forward, 1/2)
			@F others
			#F $veer(1, out) 
-- Basic
C1271796717
M1278886895
pass the ocean
	*facing_couples
		>pass thru
	+
		>quarter in
	+
		>touch
|
	*rh_wave
	!Plus
		>pass thru
	+
		>quarter in
	+
		>touch
|
	*lh_wave
	!Plus
		>left pass thru
	+
		>face in
	+
		>touch
-- Basic
C1271796717
M1295840630
extend
	*qtr_tag
		<
			@F end beaus
			#F $forward_veer(1/2, -1)
			@F end belles
			#F $forward(1/2)
			@F centers
			#F $forward(1/2)
|
	*three_qtr_tag
	!Plus
		<
			@F ends
			#F $forward(1/2)
			@F very centers
			#F $forward(1/2)
			@F $dancerC and $dancerF
			#F $forward_veer(1/2, 1)
|
	*rh_waves
	!Plus
		<
			@F lead ends
			#F $forward_veer(1/2, 1)
			@F others
			#F $forward(1/2)
|
	*lh_waves
	!Plus
		>left extend
|
	*beg_dbl_pass_thru
	!Plus
		>$forward(1/2)
|
	*lh_qtr_tag
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
M1293925703
.trailers
zoom
	*box
		<
			@F $A = leaders
			#F $arc($outside_hand, forward, 1/2)
			@F $B = trailers
			#F $nothing()
		<
			@F $A
			#F $forward(1/2)
			@F $B
			#F $forward(1/2)
	+
		<
			@F $A
			#F $forward(1/2)
			@F $B
			#F $forward(1/2)
		<
			@F $A
			#F $arc($last_hand, forward, 1/2)
|
	*tandem
		<
			@T leaders
			#F $arc($outside_hand, forward, 1/2) and $forward(1/2)
			@T trailers
			#F $forward(1/2)
		>$dont_breathe()
	+
		<
			@T $dancerA
			#F $forward(1/2)
			@T $dancerB
			#F $forward(1/2) and $arc($last_hand, forward, 1/2)
		>$dont_breathe()
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
M1294096747
trade by
	*trade_by
		<
			@F ends
			#F trade
			@F centers
			#F pass thru
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
M1294097322
slide thru
	*facing
		>pass thru
		<
			@F boys
			#T face right
			@F girls
			#T face left
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
M1279234204
quarter in
	*box
		>$face(in, $box_center)
|
	*two_x_four
		>$face(in, $split_center)
|
		>$if($face(partner, $self), $face(partner, $self), $face(in, $split_center))
-- Advanced-1
C1271796717
M1279235649
quarter out
	*box
		>$face(out, $box_center)
|
	*two_x_four
		>$face(out, $split_center)
|
		>$if($face($away_from_partner, $self), $face($away_from_partner, $self), $face(out, $split_center))
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
M1294101296
first dancer go R_L, next dancer go R_L
	*tandem
		<
			@F leads
			#F $if($1 != $2, $forward_peel(-1/2,$1), $arc($hand, $1, 1/4))
			@F trailers
			#F $if($1 != $2, $forward_peel(1/2,$2), $forward(1) and $arc($self, $2, 1/4))
-- None
C1271796717
square your set
	*infacing_ring
		>$form_set()
-- Advanced-2
C1271796717
M1294096718
single wheel
	*rh_mini_wave
	*lh_mini_wave
		>$arc($inside_hand, forward, 1/4)
		>roll
|
	*couple
		>quarter in
		>$forward_veer(1/2, -1/2)
		<
			@F $dancerA
			#F face right
			@F $dancerB
			#F face left
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
M1295215335
u-turn back
	*dancer
		>$if($can_roll(), roll twice, $arc($self, in, 1/2))
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
M1280189136
couples_face_in
	*couple
		<
			@F $dancerA
			#F $arc($inside_hand, back, 1/4)
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4)
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
M1294102499
ANYONE chain 3/4
	*squared_set_belles($1)
		<
			@F $1
			#F $forward_veer(3/2, -1) and $arc($center, forward, 1/2) and $forward(1/2)
			@F $dancerC and $dancerF
			#F face left
		<
			@F $1
			#F $arc($left_dancer, forward, 3/4) and $forward_veer(1, -1/2)
			@F $dancerC and $dancerF
			#F $arc($self, left, 3/4) and $forward_veer(1, -1/2)
			@F $dancerD and $dancerE
			#F $forward_veer(1, 1/2)
			@F others
			#F $forward_veer(-1/2, 1/2)
|
	*infacing_ring_belles($1)
		>$form_set()
		>$1 chain 3/4
-- Basic
C1273108826
M1294100465
4 ladies chain FRACTION
	*squared_set
		<
			@F boys
			#F face left
			@F girls
			#F $forward_veer(3/2, -1) and $arc($center, forward, $1 - 1/4) and $forward(1/2)
		<
			@F boys
			#F $arc($self, left, 3/4)
			@F girls
			#F $arc($left_dancer, forward, 3/4)
|
	*infacing_ring
		>square your set
		>4 ladies chain $1
-- Basic
C1273537144
M1294101566
ANYONE separate, around 2 to a line
	*couples_on_home($1)
		<
			@F $1
			#F face out
			@F others
			#F $forward(1)
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
M1277855790
wrong way promenade FRACTION
	*star
		>$form_promenade(back)
		>$arc($center, forward, $1)
		>left couples_face_in
|
		>as couples face left
		>$form_promenade(back)
		>$arc($center, forward, $1)
		>left couples_face_in
-- Mainstream
C1274135872
M1298240316
cloverleaf
	*comp_dbl_pass_thru
		>$reduce(tandem, tandem, $forward(1/2) and $arc($outside_hand, forward, 3/4))
|
	*outfacing_ends_of_columns
		>ends cloverleaf
|
	*lead_centers_of_two_x_four
		>centers cloverleaf
-- Mainstream
C1274157568
M1274566589
turn thru
	*facing
		>$forward_veer(1/2, -1/2)
		>$arc($inside_hand, forward, 1/2)
		>$forward_veer(1/2, 1/2)
|
	*rh_mini_wave
		>$arc($inside_hand, forward, 1/2)
		>$forward_veer(1/2, 1/2)
-- Mainstream
C1274167721
M1294102636
8 chain thru
		>$pull_by(right)
	+
		<
			@T ends
			#F courtesy turn
			@T centers
			#F $pull_by(left)
	+
		>$pull_by(right)
	+
		<
			@F ends
			#F courtesy turn
			@F centers
			#F $pull_by(left)
	+
		>$pull_by(right)
	+
		<
			@F ends
			#F courtesy turn
			@F centers
			#F $pull_by(left)
	+
		>$pull_by(right)
	+
		<
			@F ends
			#F courtesy turn
			@F centers
			#F $pull_by(left)
-- Mainstream
C1274167821
M1274206697
8 chain INTEGER
	*eight_chain_thru
		>$fractionalize($1 / 8, eight chain thru)
-- Mainstream
C1274206927
M1278100454
pass to the center
	*eight_chain_thru
		>pass thru
		>ends trade
|
	*rh_waves
		>pass thru
	+
		>ends trade
-- Mainstream
C1274207089
M1274207102
couples hinge
		>as couples hinge
-- Mainstream
C1274207110
M1298257070
hinge
	*rh_mini_wave
	^Preferred
		>$arc($inside_hand, forward, 1/4)
|
	*couple
	!Advanced-1
		<
			@F $dancerA
			#F $arc($inside_hand, forward, 1/4)
			@F others
			#F $arc($tail, left, 1/4)
|
	*lh_mini_wave
		>$arc($inside_hand, forward, 1/4)
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
M1298254995
spin the top
	*rh_wave
	*lh_wave
		>trade
	+
		>fan the top
|
	*facing_couples
		>step to a wave and trade
	+
		>fan the top
-- Plus
C1274213490
M1294102738
fan the top
	*rh_wave
	*lh_wave
	*rh_line
	*lh_line
		<
			@F centers
			#F cast off 3/4
			@F ends
			#F $arc($box_center, forward, 1/4)
-- Mainstream
C1274213732
M1294102798
walk and dodge
	*box
		<
			@F trailers
			#T $forward(1)
			@F others
			#T $veer(1, in)
-- Mainstream
C1274213990
M1294102842
ANYONE walk, ANYONE dodge
	*facing_couples
		<
			@F $1
			#F $forward(1)
			@F $2
			#F $veer(1, in)
-- Mainstream
C1274471953
M1296353685
ANYONE fold
	*lines
		>$1 lines_ fold
|
	*dancer_and_adjacent($1)
		<
			@F $dancerA
			#F $forward(1)
		<
			@F $dancerA
			#F $arc($inside_hand, forward, 1/2)
		>$dont_breathe()
-- Mainstream
C1274564646
M1275258876
ANYONE cross fold
	*centers_of_line($1)
	*ends_of_line($1)
		>$1 $forward(1)
		>$last_active $arc($inside_dancer, forward, 1/2)
		>$dont_breathe()
-- Mainstream
C1274565505
M1295068123
dixie style to a wave
	*facing_couples
		<
			@F belles
			#F $forward_veer(1/2, -1) and $forward_veer(1/2, 1)
			@F others
			#F $veer(1, right)
		>left turn 1/4
|
	*facing_tandems
		>leads $pull_by(right)
		>left touch 1/4
-- Mainstream
C1274566621
M1293745268
spin chain thru
	*rh_waves
		>those who can turn 1/2
	+
		>those who can left turn 3/4
	+
		>those who can turn 1/2
	+
		>those who can left turn 3/4
|
	*eight_chain_thru
		<
			@F end beaus
			#F $forward_veer(1, -1)
			@F end belles
			#F $forward(1)
			@F center beaus
			#F $veer(1, left)
		>those who can turn 1/2
	+
		>those who can left turn 3/4
	+
		>those who can turn 1/2
	+
		>those who can left turn 3/4
|
	*lh_waves
		>left spin chain thru
-- Mainstream
C1274567206
M1274567366
tag the line
	*line
		>face in
		>$forward(2)
-- Mainstream
C1274567459
M1275528956
1/2 tag
	*line
		>face in
		>$forward_veer(1, -1/2)
-- Mainstream
C1274567628
M1295068346
scoot back
	*rh_box
		<
			@F leaders
			#F $arc($inside_hand, forward, 1/2)
			@F others
			#F $forward(1/2) and $arc($inside_hand, forward, 1/2) and $forward(1/2)
|
	*qtr_tag
		<
			@F end beaus
			#F $forward_veer(1/2, -1)
			@F others
			#F $forward(1/2)
		>turn 1/2
		<
			@F end leaders
			#F $forward_veer(1/2, 1)
			@F others
			#F $forward(1/2)
|
	*lh_qtr_tag
		>left scoot back
|
	*lh_box
		>left scoot back
-- Mainstream
C1274568035
M1303021915
recycle
	*rh_wave
		<
			@F centers
			#F individually $forward_veer_face(1/2, 0, 1/2)
			@F ends
			#F individually $forward_veer(1/2, 1)
	+
		>box_ counter rotate 1/4
	+
		>roll
|
	*facing_couples
	!Advanced-2
		<
			@F $A = beaus
			#F $forward(1/2)
			@F $B = belles
			#F $forward_veer(-1/2, -1)
		<
			@F $A
			#F face right twice
			@F $B
			#F individually $forward_veer(1, -1)
|
	*lh_wave
		>left recycle
-- Advanced-2
C1274568304
M1275881520
split counter rotate
		>split counter rotate 1/4
-- None
C1274568411
M1303602423
box_ counter rotate FRACTION
	*box
		>$arc($box_center, forward, $1)
-- Advanced-2
C1274568505
M1302991601
box counter rotate
	*center_box
		>box_ counter rotate 1/4
-- Advanced-1
C1274569053
M1274648540
partner hinge
		>hinge
-- Basic
C1274647487
M1274647524
step to a left wave
		>left step to a wave
-- None
C1275458659
M1281073313
ANYONE lines_ fold
	*line
		>$1 fold
-- None
C1275507981
M1294103039
adjust to a box
	*z
		<
			@F lead centers
			#F $forward(1/2)
			@F other centers
			#F $forward(-1/2)
			@F other leads
			#F $forward(-1/2)
			@F others
			#F $forward(1/2)
|
	*z_left
		<
			@F lead centers
			#T $forward(1/2)
			@F other centers
			#T $forward(-1/2)
			@F other leads
			#T $forward(-1/2)
			@F others
			#T $forward(1/2)
|
	*end_pairs
		>$veer(1, in)
-- Plus
C1275676593
M1293745645
.tr
acey deucey
		<
			@F centers
			#F trade
			@F ends
			#F circulate
-- Plus
C1275676732
M1295075656
.left turn 3/4
all 8 spin the top
	*thar
		>trade
	+
		<
			@F centers
			#F turn 3/4
			@F ends
			#F $arc($center, forward, 1/4)
|
	*wrong_way_thar
		>trade
	+
		<
			@F centers
			#F left turn 3/4
			@F ends 
			#F $arc($center, forward, 1/4)
|
	*r_l_grand_circle
		>$forward_veer(1/2, -1/2)
		>$arc($right_hand, forward, 1/2)
		>make a wrong way thar
	+
		<
			@F centers
			#F $arc($center, forward, 3/4)
			@F ends
			#F $arc($center, forward, 1/4)
-- Plus
C1275679686
M1295075716
spread
	*tandem_couples
		<
			@F leads
			#F $veer(1, out)
			@F trailers
			#F $forward(1)
|
	*lh_wave
		>$veer(1, left)
|
	*rh_wave
		>$veer(1, right)
|
	*one_faced_line
	*lh_line
	*rh_line
	^Preferred
		>half sashay
|
	*z_in
	*z_in_left
		<
			@F centers
			#F $veer(1, out)
			@F others
			#F $forward(1)
-- Plus
C1275680111
M1295068616
chase right
	*back_to_back_couples
		<
			@F beaus
			#F $arc($inside_hand, forward, 1/2) and $forward(1)
			@F belles
			#F $arc_face($tail, right, 1/2, 1/4) and $forward_veer_face(1, 0, 1/4)
-- Plus
C1275680680
M1293745990
coordinate
	*rh_column
		>circulate 1 and 1/2
	+
		>those who can trade
	+
		<
			@F $dancerA and $dancerH
			#F $forward_veer(1, -1)
			@F $dancerC and $dancerF
			#F $forward(1/2) and $arc($inside_dancer, forward, 1/4)
		>$normalize()
|
	*lh_column
		>left coordinate
-- Plus
C1275681295
M1293750968
crossfire
	*rh_line
	*lh_line
		<
			@F ends
			#F $forward(1/2) and $arc($inside_dancer, forward, 1/2)
			@F centers
			#F trade and $forward(1/2)
|
	*inverted_line
		<
			@F $dancerA
			#F $arc($inside_dancer, forward, 1/2)
			@F $dancerB
			#F $arc($inside_dancer, forward, 1/2)
			@F $dancerC
			#F $arc($inside_hand, forward, 1/2)
			@F $dancerD
			#F $arc($line_center, forward, 1/2)
-- Plus
C1275681851
M1295842881
cut the diamond
	*diamond
		<
			@F centers
			#F $forward(1) and $arc($inside_hand, forward, 1/4)
			@F ends
			#F $veer(1/2, in) and trade
-- Plus
C1275681971
M1295076006
diamond circulate
	*diamond
		<
			@F ends
			#F $arc($inside_hand, forward, 1/4) and $forward(1/2)
			@F centers
			#F $forward(1/2) and $arc($inside_hand, forward, 1/4)
-- Plus
C1275682416
M1303024503
dixie grand
	*dixie_grand_circle
		>those who can $pull_by(right)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
|
	*wrong_way_thar
		>check a ring
		>$forward_veer(1/2, 1/2)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
|
	*ww_prom_thar
		>make a wrong way thar
		>dixie grand
|
	*ww_ring_thar
		>$forward_veer(1/2, 1/2)
	+
		>$pull_by(left)
	+
		>$pull_by(right)
-- Plus
C1275683086
M1303693381
explode
	*rh_wave
		<
			@F centers
			#F $forward(1/2)
			@F ends
			#F $forward_veer(1/2, 1)
		>$face(in, $box_center)
|
	*lh_wave
		>left explode
|
	*line
	!Advanced-1
		>$breathe(step thru)
		>quarter in
-- Plus
C1275683565
M1275683678
explode the wave
	*rh_wave
	*lh_wave
		>explode
	+
		>$pull_by(right)
-- Plus
C1275683835
M1295069316
flip the diamond
	*diamond
		<
			@F centers
			#F $forward(1) and $arc($inside_hand, forward, 1/4)
			@F ends
			#F $arc($inside_shoulder, forward, 1/2)
-- Plus
C1275683905
M1281074086
follow your neighbor
	*rh_box
	*lh_box
		<
			@F $A = trailers
			#F $forward(1/2)
			@F $B = others
			#F $forward(1/2)
		<
			@F $A
			#F cast off 1/4
			@F $B
			#F $arc($inside_hand, forward, 1/4)
		<
			@F $A
			#F trade
			@F $B
			#F u-turn back
-- Plus
C1275684175
M1275684215
grand swing thru
	*tidal
		>those who can turn 1/2
	+
		>those who can left turn 1/2
-- Plus
C1275684325
M1296353385
linear cycle
	*rh_wave
	*lh_wave
		>hinge
	+
		>leads fold
		>$forward(1)
		>$dont_breathe()
	+
		<
			@F leads
			#F $arc($inside_hand, forward, 1/2)
			@F trailers
			#F $forward(1) and u-turn back
-- Plus
C1275692475
M1295069658
load the boat
		<
			@F centers
			#F do the centers part of load the boat
			@F ends
			#F do the ends part of load the boat
-- Plus
C1275692719
M1275695194
do the centers part of load the boat
	*facing_couples
	*rh_wave
		>pass thru
	+
		>$face(out, $box_center)
	+
		>trade
	+
		>pass thru
|
	*lh_wave
		>left pass thru
	+
		>$face(out, $box_center)
	+
		>trade
	+
		>pass thru
-- Plus
C1275692851
M1296357557
do the ends part of load the boat
	*back_to_back_ends
		>$arc($inside_dancer, forward, 1/4)
		>pass thru
		>$dont_breathe()
	+
		>$arc($inside_dancer, forward, 1/4)
		>pass thru
		>$dont_breathe()
	+
		>$arc($inside_dancer, forward, 1/4)
		>pass thru
		>$dont_breathe()
	+
		>face in
|
	*infacing_ends
		>pass thru
		>$dont_breathe()
	+
		>$arc($inside_dancer, forward, 1/4)
		>pass thru
		>$dont_breathe()
	+
		>$arc($inside_dancer, forward, 1/4)
		>pass thru
		>$dont_breathe()
	+
		>face in
-- Plus
C1275695563
M1296357498
peel off
	*box
		<
			@F $A = leads
			#F $arc($outside_hand, forward, 1/2) 
			@F $B = trailers
			#F $forward(1/2)
		<
			@F $A
			#F $forward(1/2)
			@F $B
			#F $arc($self, out, 1/2)
-- Plus
C1275696053
M1298241261
peel the top
	*box
		<
			@F leads
			#F $arc($outside_hand, forward, 1/2) and $forward(1/2)
			@F trailers
			#F $forward(1/2)
	+
		>fan the top
|
	*z_in
		<
			@F leads
			#F $arc($outside_hand, forward, 1/2)
			@F others
			#F $forward(1)
		>fan the top
|
	*z_in_left
	*z_out_left
		>left peel the top
|
	*z_out
		<
			@F $dancerA and $dancerD
			#F $arc($outside_hand, forward, 1/2) and $forward(1)
		>fan the top
-- Plus
C1275697076
M1298256804
relay the deucey
	*rh_waves
		>trade
	+
		<
			@F centers
			#F $breathe(cast off 3/4)
			@F lead ends
			#F $forward(1/2) and $arc($line_center, forward, 1/4)
			@F others
			#F $forward(1/2)
		<
			@F $dancerC and $dancerF
			#F $forward(1/2)
			@F others
			#F trade
		<
			@F $dancerA and $dancerH
			#F $arc($inside_dancer, forward, 1/4) and $forward_veer(1, -1/2)
			@F $dancerC and $dancerF
			#F $forward(1/2) and $arc($line_center, forward, 1/4)
			@F others
			#F trade
		<
			@F $dancerA and $dancerH
			#T $forward(1/2)
			@F others
			#T trade
		<
			@F $dancerA and $dancerH
			#F individually $forward(1/2)
			@F $dancerD
			#F $arc($right_dancer, forward, 1/4) and $forward_veer(1/2, -1/2)
			@F $dancerB and $dancerC
			#F cast off 3/4
			@F $dancerF and $dancerG
			#F cast off 3/4
			@F $dancerE
			#F $arc($right_dancer, forward, 1/4) and $forward_veer(1/2, -1/2)
|
	*lh_waves
		>left relay the deucey
-- None
C1275697498
M1275697564
line of 6 ANYTHING
	*line_of_six
		>$1
-- Plus
C1275697918
M1275697996
single circle to a wave
	*facing
		>single circle left 1/2
		>$forward_veer(1/2, -1/2)
-- Plus
C1275776699
M1298322872
spin chain the gears, turn the stars FRACTION
	*rh_waves
		>trade
	+
		<
			@F centers
			#F cast off 3/4
			@F ends
			#F individually $forward_veer_face(0, 1, 1/2)
	+
		>very centers trade
	+
		>turn the star $1
	+
		>very centers trade
	+
		<
			@F centers
			#F cast off 3/4
			@F out-facing ends
			#F individually $forward_veer_face(-1/2, 1, 1/2)
			@F in-facing ends
			#F individually $forward_veer_face(1/2, 1, 1/2)
|
	*lh_waves
		>left spin chain the gears, turn the stars $1
|
	*eight_chain_thru
		>$breathe(step to a wave)
		>spin chain the gears, turn the stars $1
-- Advanced-2
C1275881477
M1275881534
split counter rotate FRACTION
	*two_x_four
		>box_ counter rotate $1
-- Plus
C1275885675
M1275885701
spin chain the gears
		>spin chain the gears, turn the stars 3/4
-- Plus
C1275885942
M1281146021
ping-pong circulate
	*qtr_tag
		<
			@F end beaus
			#F $forward_veer(1, -1)
			@F end belles
			#F $forward(1)
			@F very centers
			#F $forward(1) and $arc($inside_hand, forward, 1/2)
			@F others
			#F $forward(1) and $arc($inside_dancer, forward, 1/2)
|
	*lh_qtr_tag
		>left ping-pong circulate
-- None
C1275947705
M1275947909
face your partner
	*couple
	*rh_mini_wave
	*lh_mini_wave
		>$face(in, $box_center)
|
	*box
		>$face(in, $box_center)
-- Plus
C1275957536
M1293749509
ANYONE center teacup chain
	*squared_set_belles($1)
		<
			@F $1
			#F $forward_veer(3/2, -1) and $arc($center, forward, 1/2) and $forward(1/2)
			@F other belles
			#F $forward_veer_face(2, 1, 1/4)
			@F $dancerC and $dancerF
			#F face right
		<
			@F $1
			#F $arc($left_dancer, forward, 3/4)
			@F $dancerB and $dancerD and $dancerE and $dancerG
			#F turn 3/4
			@F others
			#F $arc($self, left, 3/4)
		<
			@F $dancerD and $dancerE
			#F $arc($center, forward, 5/4)
			@F $1
			#F $forward_veer(3/2, 1/2)
			@F $dancerC and $dancerF
			#F face right and $forward_veer(-1/2, -1/2)
		>turn 4/4
		<
			@F $1
			#F $arc($center, forward, 5/4)
			@F $dancerB and $dancerG
			#F $forward_veer_face(-1/2, 1/2, 1/4)
			@F other centers
			#F $forward_veer(3/2, -1/2)
		<
			@F $1 and $dancerC and $dancerF
			#F turn 4/4
			@F others
			#F $arc($left_hand, forward, 1/2)
		<
			@F $dancerD and $dancerE
			#F $forward_veer(3/2, -3/2) and $arc($center, forward, 1/2)
			@F $dancerB and $dancerG
			#F $veer(1, left)
			@F $1
			#F $forward_veer(3/2, -1/2)
		<
			@F $1 and $dancerB and $dancerG
			#F courtesy turn
			@F others
			#F as couples face right and $forward(-1)
-- Plus
C1276034057
M1278981506
trade the wave
	*rh_wave
		>$arc($inside_hand, forward, 1/4) 
		>$forward(1)
		>$arc($last_hand, forward, 1/4)
|
	*lh_wave
		>left trade the wave
-- Plus
C1277162933
M1278898462
tag the line FRACTION
FRACTION tag the line
	*line
		>$face(in, $box_center)
		>$forward(2*$1)
|
	*lines
		>$face(in, $box_center)
		>extend
	+ 4 * $1 - 1
		>extend
-- Plus
C1277163231
M1277163288
track 2
track ii
	*comp_dbl_pass_thru
		>in tandem trade
		>circulate
-- None
C1277163554
M1277163582
form stars
	*twin_diamonds
		>ends $veer(1/2, in)
-- Plus
C1277163625
M1277164092
form diamonds
	*twin_stars
		>ends $veer(1/2, out)
-- Basic
C1277163719
M1277163806
turn the star FRACTION
	*star_four
		>$arc($box_center, forward, $1)
-- Plus
C1277164403
M1277164430
spin chain and exchange the gears
		>spin chain and exchange the gears, turn the stars 3/4
-- Plus
C1277164434
M1295068681
spin chain and exchange the gears, turn the stars FRACTION
	*rh_waves
		>trade
	+
		<
			@F centers
			#F cast off 3/4
			@F ends
			#F individually $forward_veer_face(0, 1, 1/2)
	+
		>very centers trade
	+
		>turn the star $1
	+
		>last_part_of_sc_ex_gears
|
	*lh_waves
		>left spin chain and exchange the gears, turn the stars $1
-- None
C1277165414
M1293951992
last_part_of_sc_ex_gears
	*twin_rh_stars
		<
			@F $dancerD and $dancerE
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerA and $dancerH
			#F $arc($right_hand, forward, 1/4)
			@F $dancerC and $dancerF
			#F $arc($right_hand, forward, 1/4)
			@F $dancerB and $dancerG
			#F $arc($right_hand, forward, 1/4)
		<
			@F $dancerD and $dancerE
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerA and $dancerH
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerC and $dancerF
			#F $arc($right_hand, forward, 1/4)
			@F $dancerB and $dancerG
			#F $arc($right_hand, forward, 1/4)
		<
			@F $dancerD and $dancerE
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerA and $dancerH
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerC and $dancerF
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerB and $dancerG
			#F $arc($right_hand, forward, 1/4)
		<
			@F $dancerD
			#F $arc($left_hand, forward, 1/2)
			@F $dancerE
			#F $arc($left_hand, forward, 1/2)
			@F $dancerA
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerH
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerC
			#F $arc($left_hand, forward, 1/2)
			@F $dancerF
			#F $arc($left_hand, forward, 1/2)
			@F $dancerB
			#F $arc($left_two_dancers, forward, 1/4)
			@F $dancerG
			#F $arc($left_two_dancers, forward, 1/4)
|
	*twin_lh_stars
		>left last_part_of_sc_ex_gears
-- Plus
C1277417301
M1280870485
explode the line
	*one_faced_line
		<
			@F centers
			#F $forward(1/2)
			@F end beaus
			#F $forward_veer(-1/2, 1)
			@F others
			#F $forward_veer(-1/2, -1)
		>face in
	+
		>$pull_by(right)
|
	*inverted_line
		<
			@F centers
			#F $forward(1/2)
			@F end beaus
			#F $forward_veer(1/2, 1)
			@F others
			#F $forward_veer(1/2, -1)
		>face in
	+
		>$pull_by(right)
-- Advanced-1
C1278458160
M1278458184
ANYTHING and cross
		>$1
		>trailers cross
-- None
C1278458205
M1278981917
cross_
	*rh_diagonal_facing
		>$forward_veer(1, 1)
|
	*lh_diagonal_facing
		>left cross_
-- Advanced-1
C1278566335
M1278566362
ANYONE cross
		>$1 cross_
-- Advanced-1
C1278615922
M1278802936
cast a shadow
	*two_x_four
		<
			@F $A = $dancerA and $dancerE
			#F 1/2 zoom
			@F $B = out-facing centers
			#T $arc($outside_dancer, forward, 1/4)  and $forward(1)
			@F $C = in-facing centers
			#T $forward(1) and face in
			@F $D = $dancerD and $dancerH
			#F 1/2 zoom
		<
			@F $A
			#F cast off 3/4
			@F $D
			#F cast off 3/4
			@F $B
			#T $forward(1/2) and $arc($last_hand, forward, 1/2)
			@F $C
			#T $forward(3/2)
		>spread
-- Advanced-1
C1278803208
M1295843333
.xxxdddd
chain reaction, turn the star FRACTION
	*qtr_tag
		<
			@F very centers and end beaus
			#F pass thru
			@F $dancerC and $dancerF
			#F $forward_veer(3/2, 1/2) and $arc($inside_dancer, forward, 1/4)
		>$dont_breathe()
	+
		>$dancerA and $dancerD and $dancerE and $dancerH hinge
		>$dont_breathe()
	+
		<
			@F centers
			#F diamond circulate ($1 * 4) times
			@F ends
			#F trade
	+
		<
			@F center $facing_along
			#F individually $forward_veer(3/2, -1)
			@F $dancerD
			#F $arc($inside_dancer, forward, 1/4) and $forward_veer(1/2, -1/2)
			@F $dancerE
			#F $arc($inside_dancer, forward, 1/4) and $forward_veer(1/2, -1/2)
			@F others
			#F cast off 3/4
|
	*lh_qtr_tag
		>left chain reaction turn the star $1
-- Advanced-1
C1278883617
M1278883635
chain reaction
		>chain reaction, turn the star 1/4
-- Advanced-1
C1278883941
M1278883995
pass in
		>pass thru
		>face in
-- Advanced-1
C1278884182
M1278884203
pass out
		>pass thru
		>face out
-- Advanced-1
C1278884293
M1278884308
reverse swap around
		>left swap around
-- Advanced-1
C1278884313
M1278884367
swap around
	*facing_couples
		<
			@F belles
			#F $forward(1)
			@F beaus
			#F $arc($inside_hand, forward, 1/2)
-- Advanced-1
C1278884709
M1278884759
1/4 thru
		>those who can turn 1/4
	+
		>those who can left turn 1/2
-- Advanced-1
C1278884818
M1298322191
3/4 thru
three quarter thru
		>those who can turn 3/4
	+
		>those who can left turn 1/2
-- Advanced-1
C1278885465
M1278885515
pass the sea
	*facing_couples
		>pass thru
	+
		>quarter in
	+
		>step to a left wave
-- Advanced-1
C1278898626
M1278914890
lock it
	*rh_lockit
	*lh_lockit
		<
			@F centers
			#F $arc($inside_hand, forward, 1/4)
			@F ends
			#F $arc($line_center, forward, 1/4)
-- None
C1278899118
M1278899134
individually ANYTHING
	*dancer
		>$1
-- Advanced-1
C1278970344
M1278970998
transfer the column
	*rh_column
		<
			@F $A = $dancerA and $dancerG
			#F $forward(1)
			@F $B = $dancerB and $dancerH
			#F $forward(1)
			@F $C = $dancerD and $dancerE
			#F $arc($inside_dancer, forward, 1/2)
			@F $D = $dancerC and $dancerF
			#F $forward(1)
		<
			@F $A
			#F cast off 3/4
			@F $B
			#F cast off 3/4
			@F $C
			#F $forward(1)
			@F $D
			#F $arc($inside_dancer, forward, 1/2)
		<
			@F $A
			#F $forward_veer(1/2, 1/2)
			@F $B
			#F $forward_veer(1/2, 1/2)
			@F $C
			#F $arc($inside_dancer, forward, 1/4)
			@F $D
			#F $arc($inside_dancer, forward, 1/4)
|
	*lh_column
		>left transfer the column
-- Advanced-1
C1278971100
M1278972098
triple scoot
	*rh_column
		<
			@F $A = $dancerD and $dancerE
			#F $nothing()
			@F $B = others
			#F $forward(1/2)
		<
			@F $A
			#F $arc($inside_hand, forward, 1/4)
			@F $B
			#F $arc($inside_hand, forward, 1/4)
		<
			@F $A
			#F $arc($inside_hand, forward, 1/4)
			@F $B
			#F $arc($last_hand, forward, 1/4) and $forward(1/2)
|
	*lh_column
		>left triple scoot
-- Advanced-1
C1278972176
M1278972504
triple trade
	*line_of_six
		>trade
|
	*tidal
		>not very ends triple trade
-- Advanced-1
C1279000006
M1295076215
wheel thru
	*facing_couples
		<
			@F beaus
			#F $arc($right_dancer, forward, 1/4)
			@F belles
			#F $face(right, $self)
-- Advanced-1
C1279000127
M1279000702
turn and deal
	*line
		>half tag
		>$face($last, $self)
-- Advanced-1
C1279001000
M1298194756
step and slide
	*one_faced_line
		<
			@F centers
			#F $forward(1/2)
			@F end beaus
			#F $forward_veer(-1/2, 1/2)
			@F others
			#F $forward_veer(-1/2, -1/2)
|
	*rh_lockit
	*lh_lockit
		<
			@F centers
			#F $forward(1)
			@F others
			#F $veer(1, in)
|
	*inverted_line
		<
			@F centers
			#F $forward(1/2)
			@F end beaus
			#F $forward_veer(1/2, 1/2)
			@F others
			#F $forward_veer(1/2, -1/2)
|
	*duuu_line
		<
			@F centers
			#F $forward(1/2)
			@F $dancerA
			#F $forward_veer(1/2, -1/2)
			@F $dancerD
			#F $forward_veer(-1/2, -1/2)
|
	*uuud_line
		<
			@F centers
			#F $forward(1/2)
			@F $dancerA
			#F $forward_veer(-1/2, 1/2)
			@F $dancerD
			#F $forward_veer(1/2, 1/2)
-- None
C1279001163
M1279001175
divide
		>separate
-- Advanced-1
C1279002285
M1279002301
split square thru
		>split square thru 4
-- Advanced-1
C1279002309
M1279002706
split square thru INTEGER
	*split_t_bone
		>$dancerA and $dancerC $pull_by(right) and quarter in
	+
		>$pull_by(left)
	+ $1 - 2
		>quarter in
		>$pull_by(right)
-- Advanced-1
C1279004615
M1295076286
scoot and dodge
	*rh_box
		<
			@F leaders
			#F $veer(1, in)
			@F others
			#F $forward(1/2) and $arc($inside_hand, forward, 1/2) and $forward(1/2)
|
	*lh_box
		>left scoot and dodge
-- Advanced-1
C1279063011
M1279063036
mix
	*line
		>centers cross run
	+
		>centers trade
-- Advanced-1
C1279063145
M1279178067
half breed thru
	*facing_normal_couples
		>right and left thru
|
	*facing_sashayed_couples
		>pass thru
	+
		>u-turn back
|
	*half_breed_couples
		>pass thru
	+
		<
			@F $dancerC and $dancerD
			#F courtesy turn
			@F others
			#F u-turn back
-- Advanced-1
C1279156586
M1298232037
clover and ANYTHING
	*outfacing_ends_of_columns
		<
			@F $dancerA and $dancerD
			#F $arc($outside_dancer, forward, 3/4)
			@F centers
			#F $1
			@F $dancerE and $dancerH
			#F $arc($outside_dancer, forward, 3/4)
|
	*three_qtr_tag
	*lh_three_qtr_tag
		<
			@F $dancerA
			#F $arc($outside_dancer, forward, 1/2)and $forward_veer_face(1/2, 0, -1/4)
			@F $dancerG
			#F $arc($outside_dancer, forward, 1/2)and $forward_veer_face(1/2, 0, 1/4)
			@F centers
			#F $1
			@F $dancerB
			#F $arc($outside_dancer, forward, 1/2)and $forward_veer_face(1/2, 0, 1/4)
			@F $dancerH
			#F $arc($outside_dancer, forward, 1/2)and $forward_veer_face(1/2, 0, -1/4)
-- Advanced-1
C1279157005
M1295076379
cross clover and ANYTHING
	*outfacing_ends_of_columns
		<
			@F ends
			#F cross_cloverleaf
			@F centers
			#F $1
-- None
C1279157037
M1279157188
cross_cloverleaf
	*cross_clover_ends
		>$arc($inside_dancer, forward, 1/4)
		>$forward(1)
		>$arc($inside_dancer, forward, 1/2)
-- Advanced-1
C1279176657
M1279233941
cross trail thru
	*facing_couples
	*rh_waves
		>$breathe(pass thru)
		>half sashay
-- Advanced-1
C1279176762
M1279236757
right roll to a wave
	*twosome
		>leads $arc($self, right, 1/2)
		>touch
-- Advanced-1
C1279178112
M1281307954
cycle and wheel
	*uuud_line
		<
			@F beau centers
			#F $arc($outside_hand, forward, 1/4)
			@F belle centers
			#F $arc($left_hand, back, 1/4)
			@F others
			#F $arc($inside_hand, forward, 1/4)
		<
			@F $dancerA and $dancerB
			#F $forward_veer(1, -1/2)
			@F $dancerD
			#F $forward(3/2)
			@F $dancerC
			#F $arc($inside_hand, forward, 1/2) and $forward(1/2)
		<
			@F $dancerA
			#F $arc($inside_hand, forward, 1/4)
			@F $dancerB
			#F $arc($outside_hand, back, 1/4)
			@F $dancerC and $dancerD
			#F face right
|
	*uudu_line
		<
			@F $dancerA
			#F $arc($right_dancer, forward, 1/4)
			@F $dancerB
			#F face right
			@F $dancerC
			#F $arc($left_dancer, forward, 1/4)
			@F $dancerD
			#F face left
		<
			@F $dancerC
			#F $arc($inside_shoulder, forward, 1/2)
			@F $dancerD
			#F $forward_veer(1, -1/2)
			@F others
			#F $forward(1/2)
		<
			@F $dancerA
			#F $arc($right_hand, forward, 1/4)
			@F $dancerB
			#F $arc($left_hand, back, 1/4)
			@F others
			#F $forward_veer_face(1, 0, -1/4)
|
	*uduu_line
		<
			@F $dancerA
			#F $arc($inside_hand, forward, 1/4)
			@F $dancerB
			#F $arc($right_hand, forward, 1/4)
			@F $dancerC
			#F $arc($right_hand, back, 1/4)
			@F $dancerD
			#F $arc($inside_hand, forward, 1/4)
		<
			@F $dancerA
			#F $forward(1)
			@F $dancerB
			#F $arc($right_hand, forward, 1/2)
			@F others
			#F $forward_veer(1, -1/2)
		<
			@F $dancerA and $dancerB
			#F $forward_veer_face(1/2, 0, 1/4)
			@F $dancerC
			#F $arc($right_hand, back, 1/4)
			@F $dancerD
			#F $arc($left_hand, forward, 1/4)
|
	*duuu_line
		>left cycle and wheel
-- Advanced-1
C1279223638
M1280010053
double star thru
	*facing_normal_couples
		>star thru
	+
		>left star thru
-- Advanced-1
C1279232466
M1279232492
pair off
	*facing
		>face out
-- Advanced-1
C1279232731
M1279232800
square chain thru
	*facing_couples
		>$pull_by(right)
	+
		>quarter in
	+
		>left swing thru
	+
		>left turn thru
-- Advanced-1
C1279232861
M1279232886
left roll to a wave
		>$mirror(right roll to a wave)
-- Advanced-1
C1279232972
M1279233036
partner tag
	*couple
	*rh_mini_wave
	*lh_mini_wave
		>$face(in, $box_center)
		>pass thru
-- Advanced-1
C1279233227
M1279233247
triple star thru
		>those who can double star thru
	+
		>those who can star thru
-- Advanced-1
C1279233603
M1298254787
six-two acey deucey
6x2 acey deucey
	*twin_diamonds
		<
			@F very centers 
			#F trade
			@F in-facing ends
			#F $forward(2)
			@F others
			#F do your part diamond circulate
-- Advanced-1
C1279233715
M1300575241
do your part ANYTHING
		>$phantom($1)
-- Advanced-1
C1279318689
M1279318722
grand quarter thru
	*rh_column
		>hinge
	+
		>triple trade
-- Advanced-1
C1279318812
M1279318848
grand three quarter thru
grand 3/4 thru
	*rh_column
		>cast off 3/4
	+
		>triple trade
-- Advanced-1
C1279318983
M1279318996
horseshoe turn
		>clover and partner tag
-- Advanced-1
C1279322055
M1279322101
any hand 1/4 thru
	*lines
		>cast off 1/4
	+
		>centers trade
-- Advanced-1
C1279322107
M1279322139
any hand swing thru
	*line
		>trade
	+
		>centers trade
-- Advanced-1
C1279322144
M1279322170
any hand 3/4 thru
	*lines
		>cast off 3/4
	+
		>centers trade
-- Advanced-1
C1279322191
M1279322212
any hand spin the top
	*line
		>trade
	+
		>fan the top
-- Advanced-1
C1279322249
M1279322329
any hand all 8 spin the top
	*thar_star
		>trade
	+
		>fan the top
-- Advanced-1
C1279322755
M1279322918
FRACTION top
	*thar
	*wrong_way_thar
		>trade
	+ 4 * $1
		>$arc($center, forward, 1/8)
-- Advanced-1
C1279322953
M1298249023
grand follow your neighbor
	*rh_column
		<
			@F $A = $dancerD and $dancerE
			#F $arc($inside_hand, forward, 1/4)
			@F $B = others
			#F $forward(1/2)
		<
			@F $A
			#F individually $forward_veer_face(0, -3/2, 1/2)
			@F $B
			#F $breathe(cast off 3/4)
-- Advanced-1
C1279324941
M1279325539
cross over circulate
	*lines
		<
			@F leads
			#T $arc($inside_dancer, forward, 1/2)
			@F trailing beaus
			#T $forward_veer(1, 1)
			@F trailing belles
			#T $forward_veer(1, -1)
-- None
C1279325636
M1279325688
face
	*couple
		>face in
-- Basic
C1295241768
M1295241895
circle to a line
	*facing_couples
		>circle left 1/2
		>$if($closer_to_center($dancerB, $dancerC), circle_to_a_line_B, circle_to_a_line_C)
-- None
C1295241913
M1298240174
circle_to_a_line_B
	*facing_couples
		<
			@F $dancerA
			#F $forward_veer(1/2, -1)
			@F $dancerB
			#F $forward_veer(1/2, -1)
			@F $dancerC
			#F $forward_veer_face(1/2, 0, 1/2)
			@F $dancerD
			#F $forward_veer_face(1/2, -2, 1/2)
-- Mainstream
C1296355818
M1298231954
ANYONE cloverleaf
	*outfacing_ends_of_columns($1)
		>$1 $arc($outside_dancer, forward, 3/4)
|
	*lead_centers_of_two_x_four($1)
		>$1 run and roll
-- Advanced-2
C1298255351
M1303007653
pass and roll
	*single_eight_chain_thru
		>pass thru
	+
		<
			@F centers
			#F turn thru
			@F ends
			#F face right twice
		>pass thru
		>centers pass thru
		>$breathe(right roll to a wave)
-- None
C1298256104
M1298256218
step thru
	*rh_mini_wave
		>$forward_veer(1/2, 1/2)
|
	*lh_mini_wave
		>$forward_veer(1/2, -1/2)
-- Advanced-2
C1298322552
M1298322613
remake
		>those who can turn 1/4
	+
		>those who can left turn 1/2
	+
		>those who can turn 3/4
-- Advanced-2
C1302989958
M1302990544
all 4 couples ANYTHING
	*squared_set
-- Advanced-2
C1302991370
M1302991388
all 8 anything
		>all 4 couples $1
-- Advanced-2
C1302991668
M1302992330
box counter rotate FRACTION
	*center_box
		>box_ counter rotate $1
-- Advanced-2
C1302992204
M1302992525
box transfer
	*center_box
		>box_ transfer
-- None
C1302992370
M1302992935
box_ transfer
	*rh_box
		<
			@F trailers
			#F $forward(1/2) and $arc($inside_hand, forward, 3/4) and $forward(1/2)
			@F leaders
			#F $arc($inside_dancer, forward, 3/4)
|
	*lh_box
		>left box_ transfer
-- Advanced-2
C1302992468
M1302992507
split transfer
	*two_x_four
		>box_ transfer
-- Advanced-2
C1302993025
M1302993177
checkmate
checkmate the column
	*rh_column
		>circulate twice
		<
			@F $dancerA and $dancerB and $dancerG and $dancerH
			#F quarter in and $forward(1)
			@F others
			#F do your part circulate twice
|
	*lh_column
		>left checkmate
-- Advanced-2
C1302993239
M1302993656
cut the hourglass
	*hourglass
		<
			@F ends
			#F $veer(1, in) and trade
			@F others
			#F do your part hourglass circulate
-- Advanced-2
C1302993530
M1302993916
hourglass circulate
	*hourglass
		<
			@F very centers
			#F $veer(1, out) and $forward(1/2)
			@F other centers
			#F $forward(1) and $arc($inside_hand, forward, 1/4)
			@F in-facing ends
			#F $veer(1, in) and $forward(1./2)
			@F others
			#F $arc($inside_hand, forward, 1/4) and $forward(1)
-- Advanced-2
C1303000183
M1303000298
diamond chain thru
	*twin_diamonds
		>diamond circulate
	+
		>very centers trade
	+
		>centers cast off 3/4
-- Advanced-2
C1303000632
M1303000770
flip the hourglass
	*hourglass
		<
			@F ends
			#F $arc($inside_hand, forward, 1/2)
			@F centers
			#F do your part hourglass circulate
-- Advanced-2
C1303000821
M1303002271
in-roll circulate
	*rh_tandem_ended_lines
		<
			@F $A = in-facing ends
			#F $forward_veer(1/2, -1/2)
			@F $B = other ends
			#F $forward_veer_face(0, 1/2, 1/4)
			@F $C = center in-facing belles
			#T $forward_veer_face(0, -1/2, -1/4)
			@F $D = center out-facing belles
			#T $forward_veer_face(0, 1/2, 1/4)
			@F $E = center in-facing beaus
			#T $forward_veer_face(0, -1/2, -1/4)
			@F $F = center out-facing beaus
			#T $forward_veer_face(0, 1/2, 1/4)
	+
		<
			@F $A
			#F $forward_veer(1/2, 1/2)
			@F $B
			#F $forward_veer_face(1/2, 0, 1/4)
			@F $C
			#T $forward_veer_face(1/2, 0, -1/4)
			@F $D
			#T $forward_veer_face(1/2, 0, 1/4)
			@F $E
			#T $forward_veer_face(1/2, 0, -1/4)
			@F $F
			#T $forward_veer_face(1/2, 0, 1/4)
|
	*lh_tandem_ended_lines
		>left in-roll circulate
-- Advanced-2
C1303002967
M1303604188
mini-busy
	*rh_lines
		<
			@F $A = leaders
			#F $forward_veer_face(1/2, 0, 1/4)
			@F $B = trailers
			#F $forward(1/2)
	+
		<
			@F $A
			#F individually $forward_veer(1, -1/2)
			@F $dancerC and $dancerF
			#F hinge
			@F others
			#F $veer(1/2, in)
	+
		<
			@F $A
			#F individually quarter right
			@F $B
			#F flip the diamond
|
	*lh_lines
		>left mini-busy
-- Advanced-2
C1303003323
M1303006337
motivate
	*lines
		>circulate
	+
		<
			@F centers
			#F cast off 3/4
			@F ends
			#F 1/2 circulate
	+
		<
			@F centers
			#F form a star and $arc($center, forward, 1/2)
			@F ends
			#F trade
	+
		<
			@F very end belles
			#T individually left turn_corner
			@F very end beaus
			#T turn_corner
			@F beau facing_along centers
			#T individually $forward_veer(1, -1)
			@F belle facing_along centers
			#T individually $forward_veer(1, 1)
			@F others
			#F cast off 3/4
-- None
C1303003680
M1303004606
form a star
	*diamond
		<
			@F ends
			#F $veer(1/2, in)
			@F centers
			#F $nothing()
-- None
C1303006138
M1303006263
turn_corner
	*dancer
		>$arc($right_dancer, forward, 1/4)
		>$forward_veer(1/2, -1/2)
-- Advanced-2
C1303006410
M1303007328
out-roll circulate
	*rh_tandem_ended_lines
		<
			@F $A = out-facing ends
			#F turn_corner
			@F $B = other ends
			#F $forward_veer_face(0, 1/2, 1/4)
			@F $C = center in-facing belles
			#T $forward_veer_face(0, 1/2, 1/4)
			@F $D = center out-facing belles
			#T $forward_veer_face(0, -1/2, -1/4)
			@F $E = center in-facing beaus
			#T $forward_veer_face(0, 1/2, 1/4)
			@F $F = center out-facing beaus
			#T $forward_veer_face(0, -1/2, -1/4)
	+
		<
			@F $A
			#F turn_corner_2
			@F $B
			#F $forward_veer_face(1/2, 0, 1/4)
			@F $C
			#T $forward_veer_face(1/2, 0, 1/4)
			@F $D
			#T $forward_veer_face(1/2, 0, -1/4)
			@F $E
			#T $forward_veer_face(1/2, 0, 1/4)
			@F $F
			#T $forward_veer_face(1/2, 0, -1/4)
|
	*lh_tandem_ended_lines
		>left out-roll circulate
-- None
C1303007231
M1303007319
turn_corner_2
	*dancer
		>$forward(1/2)
		>$arc($right_dancer, forward, 1/4)
-- Advanced-2
C1303007779
M1303020748
pass and roll your neighbor
	*single_eight_chain_thru
		>pass thru
	+
		<
			@F centers
			#F turn thru
			@F others
			#F face right twice
		>$breathe(touch)
		>follow your neighbor
-- Advanced-2
C1303020829
M1303021195
peel and trail
	*tandem_couples
	*rh_box
	*lh_box
		<
			@F leaders
			#F $arc($outside_hand, forward, 1/2) and $forward(1/2)
			@F trailers
			#F $forward(1/2) and left trade
|
	*z_out
	*z_out_left
		<
			@F centers
			#F trade
			@F others
			#F $arc($outside_hand, forward, 1/2) and $forward(1/2)
-- Advanced-2
C1303022138
M1303022174
grand remake
	*rh_column
		>hinge
	+
		>triple trade
	+
		>cast off 3/4
-- Advanced-2
C1303023129
M1303023409
remake the thar
	*thar
		>left hinge
	+
		>trade
	+
		>left cast off 3/4
|
	*wrong_way_thar
		>hinge
	+
		>left trade
	+
		>cast off 3/4
-- Challenge-1
C1303083823
M1303084138
weave
	*rh_box
	*lh_box
		>$breathe(hinge)
		>centers trade
-- Advanced-2
C1303083841
M1303083923
scoot and weave
	*rh_box
	*lh_box
		>scoot back
	+
		>weave
-- Advanced-2
C1303084204
M1303084391
scoot chain thru
	*rh_waves
		>1/2 split circulate
		>centers swing
		>centers slip
		>centers swing
		>drop in
|
	*lh_waves
		>left scoot chain thru
-- Advanced-2
C1303084369
M1303084421
swing
	*line
		>trade
-- Advanced-2
C1303084454
M1303084477
slip
	*line
		>centers trade
-- Challenge-2
C1303084504
M1303084557
drop in
	*diamond
		<
			@F centers
			#F $forward(1/2)
			@F ends
			#F $arc($inside_hand, forward, 1/4)
-- Advanced-2
C1303085847
M1303086024
slide
	*line
		>slide_each_pair
-- None
C1303086034
M1303086138
slide_each_pair
	*couple
		>half sashay
|
	*rh_mini_wave
		>$forward_veer(-1/2, 1/2)
		>$forward_veer(1/2, 1/2)
|
	*lh_mini_wave
		>$forward_veer(-1/2, -1/2)
		>$forward_veer(1/2, -1/2)
-- Advanced-2
C1303086367
M1303086400
slither
	*line
		>centers slide_each_pair
-- Advanced-2
C1303086650
M1303087331
spin the windmill, ANYDIRECTION
	*two_four_two_lines
		<
			@F centers
			#F swing and slip and cast off 3/4
			@F ends
			#F face $1 and circulate twice
|
	*beg_dbl_pass_thru
	*trade_by
		>centers touch
		>spin the windmill $1
-- Advanced-2
C1303087113
M1303087285
spin the windmill, outsides ANYTHING
	*two_four_two_lines
		<
			@F centers
			#F swing and slip and cast off 3/4
			@F ends
			#F $1
-- Advanced-2
C1303087603
M1303087628
zig
	*dancer
		>face right
-- Advanced-2
C1303087638
M1303087658
zag
	*dancer
		>face left
-- Advanced-2
C1303087663
M1303087750
zig zag
		<
			@F leaders
			#F face right
			@F trailers
			#F face left
-- Advanced-2
C1303087760
M1303087790
zig zig
		<
			@F leaders
			#F face right
			@F trailers
			#F face right
-- Advanced-2
C1303087794
M1303087825
zag zag
		<
			@F leaders
			#F face left
			@F trailers
			#F face left
-- Advanced-2
C1303087830
M1303087855
zag zig
		<
			@F leaders
			#F face left
			@F trailers
			#F face right
-- Advanced-2
C1303097673
M1303097769
ANYONE split square chain thru
	*couples_on_home($1)
		>$1 move in
		>split square chain thru
-- Advanced-2
C1303097931
M1303601898
split square chain thru
	*split_t_bone
		>$dancerA and $dancerC $pull_by(right)
	+
		>$dancerA and $dancerC quarter  in
	+
		>left swing thru
	+
		>left turn thru
-- Advanced-1
C1303602825
M1303602967
ANYONE split square thru
	*couples_on_home($1)
		>$1 move in
		>split square thru
-- Advanced-1
C1303602981
M1303603043
ANYONE split square thru INTEGER
	*couples_on_home($1)
		>$1 move in
		>split square thru $2
-- Advanced-2
C1303606477
M1303606567
switch the wave
	*rh_wave
	*lh_wave
		<
			@F centers
			#F $arc($outside_hand, forward, 1/2)
			@F ends
			#F $arc($inside_dancer, forward, 1/2)
-- Advanced-2
C1303608667
M1303609158
switch to a diamond
	*line
		<
			@F centers
			#F $arc($outside_hand, forward, 1/2) and $veer(1/2, in)
			@F ends
			#F $arc($inside_hand, forward, 1/4) and $forward(1)
-- Advanced-2
C1303692277
M1303692444
switch to an hourglass
	*rh_waves
	*lh_waves
		<
			@F centers
			#F $arc($outside_hand, forward, 1/2)
			@F in-facing ends
			#F $veer(1, in) and $forward(1/2)
			@F others
			#F $arc($inside_hand, forward, 1/4) and $forward(1)
-- Plus
C1303693396
M1303746088
ANYONE explode
	*rh_split_mini_wave($1)
		<
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4) and $forward(1/2)
			@F $dancerC
			#F $forward_veer_face(1/2, 0, -1/4)
|
	*lh_split_mini_wave($1)
		<
			@F $dancerB
			#F $arc($inside_hand, forward, 1/4) and $forward(1/2)
			@F $dancerC
			#F $forward_veer_face(1/2, 0, 1/4)
-- Advanced-2
C1303746156
M1303746322
trade circulate
	*rh_waves
	*lh_waves
		>cross over circulate
|
	*rh_lines
		<
			@F leaders
			#F trade
			@F others
			#F $forward_veer(1, 2)
|
	*lh_lines
		>left trade circulate
-- Advanced-2
C1303777685
M1304197787
trail off
	*box
		<
			@F $A = leaders
			#F $arc($inside_dancer, forward, 1/2) 
			@F $B = trailers
			#F $forward(1/2)
		<
			@F $A
			#F $forward(1/2)
			@F $B
			#F $arc($inside_hand, forward, 1/2)
|
	*z_left
		>left trail off
|
	*z_tandem_out
		<
			@F leaders
			#F $arc($inside_dancer, forward, 1/2) and $forwaard(1)
			@F trailers
			#F trade
|
	*z_tandem_in
		<
			@F leaders
			#F $arc($inside_dancer, forward, 1/2)
			@F trailers
			#F $forward(1) and trade
@@
++
C1276803212
M1277106082
.$last_active
those dancers
++
C1276803212
M1277100631
.boys
boys
gents
men
++
C1276803212
M1277100621
.girls
girls
ladies
women
++
C1276803212
.heads
heads
++
C1276803212
.sides
sides
++
C1276803212
.others
others
++
C1276803212
.centers
centers
++
C1276803212
.ends
ends
++
C1276803212
.leaders
leaders
++
C1276803212
.trailers
trailers
++ Advanced-1
C1276803212
.beaus
beaus
++ Advanced-1
C1276803212
.belles
belles
++
C1276803212
M1277093957
.$very_centers
very centers
++
C1276803212
M1277093963
.$very_ends
very ends
++
C1276803212
.$facing_across
$facing_across
++
C1276803212
M1276809134
.$facing_along
$facing_along
++
C1276803212
.leaders & $1
lead ANYONE
++
C1276803212
.ends & $1
end ANYONE
++
C1276803212
.centers & $1
center ANYONE
++
C1277094011
M1277094024
.heads & $1
head ANYONE
++
C1277100361
M1277100377
.sides & $1
side ANYONE
++
C1277101235
M1277101249
.$1 | $2
ANYONE and ANYONE
++
C1277101317
M1277101332
.boys & $1
boy ANYONE
++
C1277101336
M1277101361
.girls & $1
girl ANYONE
++
C1277158600
M1277158612
.heads | sides
all
++
C1277239424
M1277239441
.trailers & $1
trailing ANYONE
++
C1278029367
M1278029411
.$in_facing & $1
in-facing ANYONE
++
C1278029418
M1278029434
.$out_facing & $1
out-facing ANYONE
++
C1278029445
M1278101219
.$in_facing
in-facing dancers
++
C1278029461
M1278101226
.$out_facing
out-facing dancers
++
C1278972287
M1278972652
.!$1
not ANYONE
++ Advanced-1
C1279063457
M1279063479
.belles & $1
belle ANYONE
++ Advanced-1
C1279063485
M1279063498
.beaus & $1
beau ANYONE
++
C1293747583
M1293747621
.others & $1
other ANYONE
++
C1294097444
M1301808186
.$facing_across & $1
cross_facing ANYONE
++ None
C1303004352
M1303004380
.$facing_along & $1
facing_along ANYONE
++ None
C1303004976
M1303005028
.$very_centers & $1
very center ANYONE
++ None
C1303005037
M1303005054
.$very_ends & $1
very end ANYONE
%%
=squared_set
*C1274473265
. . av . av

a> . . . . . a<

a> . . . . . a<

. . a^ . a^
=squared_set_belles
*C1274473265
. . dv . nv

n> . . . . . n<

n> . . . . . n<

. . n^ . d^
=opposite_homes
*C1274473265
a> . . . . . a<

a> . . . . . a<
=couples_on_home
*C1274473265
. . d| . d|

n? . . . . . n?

n? . . . . . n?

. . d| . d|
=lines
*C1274473265
a| . a| . a| . a|

a| . a| . a| . a|
=rh_waves
*C1274473265
a^ . av . a^ . av

a^ . av . a^ . av
=column
*C1274473265
a- . a- . a- . a-

a- . a- . a- . a-
=facing_lines
*C1274473265
av . av . av . av

a^ . a^ . a^ . a^
=beg_dbl_pass_thru
*C1274473265
a> . a> . a< . a<

a> . a> . a< . a<
=comp_dbl_pass_thru
*C1274473265
a< . a< . a> . a>

a< . a< . a> . a>
=diamond
*C1274473265
. e-

c| . c|

. e-
=box
*C1274473265
a? . a?

a? . a?
=rh_box
*C1274473265
a^ . av

a^ . av
=lh_box
*C1274473265
av . a^

av . a^
=facing_belles
*C1274473265
dv . nv

n^ . d^
=facing_couples
*C1274473265
av . av

a^ . a^
=tandem_couples
*C1274473265
a^ . a^

a^ . a^
=one_x_four
*C1274473265
e? . c? . c? . e?
=two_x_four
*C1274473265
e? . c? . c? . e?

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

av . av

a^ . a^

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

c| . C| . C| . c|

. . e| . e|
=twin_diamonds
*C1274473265
. e- . . . e-

c| . C| . C| . c|

. e- . . . e-
=pt_to_pt_diamonds
*C1274473265
. . c- . . . . . c-
E| . . . C| . C| . . . E|
. . c- . . . . . c-
=qtr_tag
*C1274473265
. . av . av

a^ . av . a^ . av

. . a^ . a^
=lh_qtr_tag
*C1274473265
*M1275885986
. . av . av

av . a^ . av . a^

. . a^ . a^
=three_qtr_tag
*C1274473265
*M1279239128
. . d^ . d^

n^ . nv . n^ . nv

. . dv . dv
=lh_three_qtr_tag
*C1274473265
*M1275885994
. . a^ . a^

av . a^ . av . a^

. . av . av
=prom_star
*C1274473265
. . . . e?

. . . . c?

e? . c? . . . c? . e?

. . . . c?

. . . . e?
=promenade
*C1274473265
. . . . a<

. . . . a<

av . av . . . a^ . a^

. . . . a>

. . . . a>
=reverse_promenade
*C1274473265
. . . . a>

. . . . a>

a^ . a^ . . . av . av

. . . . a<

. . . . a<
=thar_star
*C1274473265
. . . e-

. . . c-
e| . c| . c| . e|
. . . c-

. . . e-
=thar
*C1274473265
. . . a<

. . . a>
av . a^ . av . a^
. . . a<

. . . a>
=wrong_way_thar
*C1274473265
. . . a>

. . . a<
a^ . av . a^ . av
. . . a>

. . . a<
=star
*C1274473265
. . . a>

. . . a>
a^ . a^ . av . av
. . . a<

. . . a<
=lh_star
*C1274473265
. . . a<

. . . a<
av . av . a^ . a^
. . . a>

. . . a>
=start_tandem_split_two
*C1274473265
. . . . n-
d> . d>
. . . . n-
=start_couple_split_two
*C1274473265
d> . n-

d> . n-
=lead_centers_of_two_x_four
*C1274473265
n? . d^ . d^ . n?

n? . dv . dv . n?
=lead_centers_trailing_ends
*C1274473265
n> . d^ . d^ . n<

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

a> . a< . a> . a<
=rh_column
*C1274473265
a> . a> . a> . a>

a< . a< . a< . a<
=lh_column
*C1274473265
a< . a< . a< . a<

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

a| . . . . . a|
=cross_pair
*C1274473265
a| . . . a|
=couple
*C1274473265
*M1281227548
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
*M1274568710
d| . n?
=boy_facing_girl
*C1274564477
*M1274564499
b> . g<
=facing_tandems
*C1274565545
*M1274565564
a> . a> . a< . a<
=lh_waves
*C1274567016
*M1274567050
av . a^ . av . a^

av . a^ . av . a^
=outfacing_ends_of_columns
*C1274647658
*M1275002132
d< . n? . n? . d>

d< . n? . n? . d>
=outfacing_ends_of_triple_lines
*C1274647743
*M1274647767
a< . . . a>

a< . . . a>
=lh_lines
*C1274648941
*M1274648968
a< . a<

a< . a<

a> . a>

a> . a>
=z
*C1275508044
*M1275508426
a- . c-

. . c- . a-
=z_left
*C1275508134
*M1275508414
. . c- . a-

a- . c-
=rh_lines
*C1275677705
*M1275677731
a^ . a^ . av . av

a^ . a^ . av . av
=back_to_back_couples
*C1275680156
*M1275680174
a^ . a^

av . av
=sausage
*C1275681090
*M1303002478
. . a- . C- . a-
E| . . . . . . . E|
. . a- . C- . a-
=dixie_grand_circle @ring
*C1275682330
*M1275682380
a- . a- . a- . a> . a< . a- . a- . a- . \
=back_to_back_ends
*C1275692930
*M1275692959
a^ . . . . . a^

av . . . . . av
=infacing_ends
*C1275692964
*M1275692979
av . . . . . av

a^ . . . . . a^
=I
*C1275695306
*M1275695395
e? . . . . . . . . . e?
. . c? . C? . C? . c?
e? . . . . . . . . . e?
=line_of_six
*C1275697470
*M1275697548
e| . c| . C| . C| . c| . e|
=center_box
*C1277159940
*M1277159973
i? . a? . a? . i?

i? . a? . a? . i?
=z_out
*C1277163659
*M1279001554
a< . a-

. . a- . a>
=star_four
*C1277163754
*M1277163775
. a-
a| . a|
. a-
=twin_stars
*C1277163947
*M1277851105
. e- . . . e-
c| . C| . C| . c|
. e- . . . e-
=twin_rh_stars
*C1277165675
*M1277165700
. a> . . . a>
a^ . av . a^ . av
. a< . . . a<
=twin_lh_stars
*C1277165708
*M1277165733
. a< . . . a<
av . a^ . av . a^
. a> . . . a>
=infacing_ring_belles @ring
*C1277247203
*M1277247279
dv . nv . nv . nv . dv . nv . nv . nv . \
=sausage_line
*C1277270126
*M1277270178
. . a| . a| . a|
a- . . . . . . . a-
. . a| . a| . a|
=lh_diagonal_facing
*C1278458255
*M1278566301
. . a<

a>
=rh_diagonal_facing
*C1278458332
*M1278566306
a>

. . a<
=z_in
*C1278802456
*M1278802526
a> . a-

. . a- . a<
=z_in_left
*C1278802499
*M1278802518
. . a- . a<

a> . a-
=three_x_one_diamond
*C1278803942
*M1278804020
. . . . . . C?
E? . e? . c? . . . c? . e? . E?
. . . . . . C?
=six_cross_two
*C1278883527
*M1278883603
. . . . . c?
E? . e? . c? . c? . e? . E?
. . . . . c?
=rh_lockit
*C1278914813
*M1278914835
a| . av . a^ . a|
=lh_lockit
*C1278914843
*M1278914880
a| . a^ . av . a|
=z_out_left
*C1279001490
*M1279001541
. . a- . a>

a< . a-
=split_t_bone
*C1279002388
*M1279002425
av . a<

a^ . a<
=cross_clover_ends
*C1279157057
*M1279157094
a< . . . . . a>

a< . . . . . a>
=twosome_pair
*C1279176838
*M1279176893
a| . a|

a| . a|
=facing_normal_couples
*C1279177695
*M1279177718
gv . bv

b^ . g^
=facing_sashayed_couples
*C1279177721
*M1279177744
bv . gv

g^ . b^
=half_breed_couples
*C1279177749
*M1279177772
bv . gv

b^ . g^
=uuud_line
*C1279178260
*M1279178337
a^ . a^ . a^ . av
=uudu_line
*C1279178341
*M1279178356
a^ . a^ . av . a^
=uduu_line
*C1279178357
*M1279178370
a^ . av . a^ . a^
=duuu_line
*C1279178370
*M1279178392
av . a^ . a^ . a^
=line_between_pairs
*C1280010531
*M1280182388
. . . E-

. . . e-

c| . C| . C| . c|

. . . e-

. . . E-
=box_between_pairs
*C1280182540
*M1280182603
. . . . c? . c?
E? . e? . . . . . e? . E?
. . . . c? . c?
=box_between_i_pairs
*C1280182798
*M1280182839
. . . . a? . a?
i? . i? . . . . . i? . i?
. . . . a? . a?
=centers_facing
*C1292786428
*M1293672819
i? . av . av . i?

i? . a^ . a^ . i?
=centers_facing_out
*C1296355438
*M1296355479
i? . a^ . a^ . i?

i? . av . av . i?
=triple_diamonds
*C1298241809
*M1298241895
. . . . . c?

E? . e? . C? . C? . e? . E?

. . . . . c?
=prom_thar
*C1298257159
*M1298257199
. . . . a>

. . . . a<

a^ . av . . . a^ . av

. . . . a>

. . . . a<
=hourglass
*C1302993280
*M1302993345
. . . c?
e? . . . . . e?
. . C? . C?
e? . . . . . e?
. . . c?
=rh_tandem_ended_lines
*C1303000894
*M1303001249
a^ . a| . a| . av

a^ . a| . a| . av
=lh_tandem_ended_lines
*C1303000933
*M1303001225
av . a| . a| . a^

av . a| . a| . a^
=three_x_one_star
*C1303004131
*M1303004260
. . . . . c?
E? . e? . c? . c? . e? . E?
. . . . . c?
=single_eight_chain_thru
*C1303007430
*M1303007480
a> . a< . a> . a<
=ww_prom_thar
*C1303023839
*M1303023893
. . . . a>

. . . . a<

a^ . av . . . a^ . av

. . . . a>

. . . . a<
=ww_ring_thar @ring
*C1303024299
*M1303024372
a> . . . a> . . . a> . . . a>

a< . . . a< . . . a< . . . a<
=lh_split_mini_wave
*C1303693147
*M1303693345
. . . . n?
dv . d^
. . . . n?
=rh_split_mini_wave
*C1303693244
*M1303693325
. . . . n?
d^ . dv
. . . . n?
=z_tandem_out
*C1304187884
*M1304187937
a< . a<

. . a> . a>
=z_tandem_in
*C1304187950
*M1304187972
a> . a>

. . a< . a<
