# === run_all_byname.do ===
transcript on

set	ROOT	[pwd]
set	IN_DIR	"$ROOT/data/input/"
set	OUT1	"$ROOT/data/output/stage1/"
set	OUT2	"$ROOT/data/output/stage2/"
set	OUT3	"$ROOT/data/output/stage3/"
file	mkdir	$OUT1
file	mkdir	$OUT2
file	mkdir	$OUT3

# TB/DUT generici po potrebi
set	G_LUT_ID	3

# pročitaj N iz N_000.txt
set	COUNT_FILE	"${IN_DIR}N_000.txt"
if {![file exists $COUNT_FILE]} {
	puts	"Greška: nema $COUNT_FILE"
	quit	-f
}
set	fh	[open $COUNT_FILE r]
set	content	[read $fh]
close	$fh
set	NSETS	[expr {int([string trim $content])}]
if {$NSETS <= 0} {
	puts	"Greška: N u $COUNT_FILE je ≤ 0 (pročitano: '$content')"
	quit	-f
}

# širina indeksa (000, 001, …)
set	PAD_WIDTH	3

proc	base_no_ext {path} { return [file rootname [file tail $path]] }

# izlazna imena iz I-baze, uz stage podfoldere
proc	make_outputs {out1 out2 out3 base_i} {
	set	o1i	[string map {"xin_i" "xout_i_stage1"} $base_i]
	set	o1q	[string map {"xin_i" "xout_q_stage1"} $base_i]
	set	o2i	[string map {"xin_i" "xout_i_stage2"} $base_i]
	set	o2q	[string map {"xin_i" "xout_q_stage2"} $base_i]
	set	o3	[string map {"xin_i" "xout_stage3"}   $base_i]
	return	[list \
		"$out1$o1i.txt" \
		"$out1$o1q.txt" \
		"$out2$o2i.txt" \
		"$out2$o2q.txt" \
		"$out3$o3.txt" ]
}

for {set i 0} {$i < $NSETS} {incr i} {
	set	idx_str	[format "%0*d" $PAD_WIDTH $i]

	# dozvoli sufikse (npr. _f0.123): traži po šablonu
	set	pattern	"xin_i*_${idx_str}*.txt"
	set	cands	[lsort [glob -nocomplain -directory $IN_DIR $pattern]]

	if {[llength $cands] == 0} {
		puts	"Upozorenje: nema I-fajla za indeks $idx_str (šablon: $pattern) — preskačem."
		continue
	}
	if {[llength $cands] > 1} {
		puts	"Napomena: više I-fajlova za $idx_str, uzimam prvi: [file tail [lindex $cands 0]]"
	}
	set	fi	[lindex $cands 0]
	set	base_i	[base_no_ext $fi]

	# izvedi Q fajl: zameni 'xin_i' → 'xin_q' u tail delu
	set	q_tail	[string map {"xin_i" "xin_q"} [file tail $fi]]
	set	fq	"$IN_DIR$q_tail"
	if {![file exists $fq]} {
		puts	"Upozorenje: nema Q para za [file tail $fi] (traženo: $q_tail) — preskačem."
		continue
	}

	# kreiraj izlazne putanje u stage podfolderima
	set	OUTS	[make_outputs $OUT1 $OUT2 $OUT3 $base_i]
	lassign	$OUTS OUT_I_S1 OUT_Q_S1 OUT_I_S2 OUT_Q_S2 OUT_S3

	puts	"=== RUN [$i/$NSETS]: [file tail $fi] → [file tail $OUT_S3] ==="

	vsim	-c	-voptargs=+acc	work.tb_rfTransmitter \
		-gLUT_ID=$G_LUT_ID \
		-gIN_FILE_I={"$fi"} \
		-gIN_FILE_Q={"$fq"} \
		-gOUT_FILE_I_STAGE1={"$OUT_I_S1"} \
		-gOUT_FILE_Q_STAGE1={"$OUT_Q_S1"} \
		-gOUT_FILE_I_STAGE2={"$OUT_I_S2"} \
		-gOUT_FILE_Q_STAGE2={"$OUT_Q_S2"} \
		-gOUT_FILE_STAGE3={"$OUT_S3"}

	run	-all
	quit	-sim
}

puts	"=== run_all_byname.do: done (N=$NSETS) ==="
