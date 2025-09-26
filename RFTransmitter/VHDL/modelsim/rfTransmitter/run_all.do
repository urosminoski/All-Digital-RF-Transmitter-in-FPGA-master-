# === run_all.do (run-only; pokreće tb_rfTransmitter kroz sve (I,Q) parove) ===
transcript on

# ROOT = .../RFTransmitter
set	THISDIR	[file dirname [file normalize [info script]]]
set	ROOT	[file normalize [file join $THISDIR ../../../]]

# I/O folderi
set	IN_DIR	"$ROOT/data/input/tmp/"
set	OUT1	"$ROOT/data/output/tmp/stage1/"
set	OUT2	"$ROOT/data/output/tmp/stage2/"
set	OUT3	"$ROOT/data/output/tmp/stage3/"
file	mkdir	$OUT1
file	mkdir	$OUT2
file	mkdir	$OUT3

# work LIB mora da postoji (prethodno kompajliraj RTL i TB u 'work')
if {![file isdirectory work]} {
	puts	"Greška: library 'work' ne postoji (kompajliraj projekat pa pokreni ovu skriptu)."
	quit	-f
}
vmap	work	work

# Parametri TB/DUT (po potrebi promeni)
set	G_LUT_ID	3
set G_KERNEL_ID 3
set G_DTYPE 0

# Helperi
proc	base_no_ext {path} { return [file rootname [file tail $path]] }
proc	make_outputs {out1 out2 out3 base_i} {
	# base_i npr: xinI_000_f0p010
	set	o1i	[string map {"xinI" "xout_i_stage1"} $base_i]
	set	o1q	[string map {"xinI" "xout_q_stage1"} $base_i]
	set	o2i	[string map {"xinI" "xout_i_stage2"} $base_i]
	set	o2q	[string map {"xinI" "xout_q_stage2"} $base_i]
	set	o3	[string map {"xinI" "xout_stage3"}   $base_i]
	return	[list \
		"$out1$o1i.txt" \
		"$out1$o1q.txt" \
		"$out2$o2i.txt" \
		"$out2$o2q.txt" \
		"$out3$o3.txt" ]
}

# Učitaj N (maks broj parova koje ćemo obraditi)
set	COUNT_FILE	"${IN_DIR}N_000.txt"
if {![file exists $COUNT_FILE]} {
	puts	"Greška: ne postoji $COUNT_FILE"
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

# Nađi sve I-fajlove (xinI_*.txt) i upari sa Q (xinQ_*.txt)
set	I_LIST	[lsort [glob -nocomplain -directory $IN_DIR xinI_*.txt]]

# Napravi listu {index  ifile  qfile  base_i}; indeks čitamo DECIMALNO (izbegavamo oktalnu zamku)
set	PAIRS	{}
foreach fi $I_LIST {
	set	tailI	[file tail $fi]

	# očekujemo format: xinI_([0-9]+)_
	if {![regexp {^xinI_([0-9]+)_} $tailI -> idx_str]} {
		puts	"Upozorenje: ne mogu da izdvojim indeks iz $tailI — preskačem."
		continue
	}
	# decimalni parse (npr. '008' -> 8)
	if {[scan $idx_str "%d" idx] != 1} {
		puts	"Upozorenje: indeks '$idx_str' iz $tailI nije decimalan — preskačem."
		continue
	}

	# mapiraj Q fajl
	set	q_tail	[string map {"xinI_" "xinQ_"} $tailI]
	set	fq	"$IN_DIR$q_tail"
	if {![file exists $fq]} {
		puts	"Upozorenje: nema Q para za $tailI (traženo: $q_tail) — preskačem."
		continue
	}

	lappend	PAIRS [list $idx $fi $fq [base_no_ext $fi]]
}

# Sortiraj po indexu i skrati na N parova
set	PAIRS	[lsort -integer -index 0 $PAIRS]
if {[llength $PAIRS] > $NSETS} {
	set	PAIRS	[lrange $PAIRS 0 [expr {$NSETS-1}]]
}

# Glavna petlja
set	total	[llength $PAIRS]
if {$total == 0} {
	puts	"Upozorenje: nema validnih (I,Q) parova u $IN_DIR (xinI_*.txt / xinQ_*.txt)."
	puts	"=== run_all.do: done (processed 0 / requested $NSETS) ==="
	quit	-f
}

set	i	0
foreach rec $PAIRS {
	incr	i
	lassign	$rec idx fi fq base_i

	set	OUTS	[make_outputs $OUT1 $OUT2 $OUT3 $base_i]
	lassign	$OUTS OUT_I_S1 OUT_Q_S1 OUT_I_S2 OUT_Q_S2 OUT_S3

	set	prog	[format {%d/%d} $i $total]
	puts	"=== RUN $prog idx=$idx : [file tail $fi] → [file tail $OUT_S3] ==="

	# --- umesto dosadašnjeg vsim bloka ---
	set vsim_args [list \
		vsim -c -voptargs=+acc work.tb_rfTransmitter \
        -gDTYPE=$G_DTYPE \
		-gLUT_ID=$G_LUT_ID \
        -gKERNEL_ID=$G_KERNEL_ID \
		-gIN_FILE_I="$fi" \
		-gIN_FILE_Q="$fq" \
		-gOUT_FILE_I_STAGE1="$OUT_I_S1" \
		-gOUT_FILE_Q_STAGE1="$OUT_Q_S1" \
		-gOUT_FILE_I_STAGE2="$OUT_I_S2" \
		-gOUT_FILE_Q_STAGE2="$OUT_Q_S2" \
		-gOUT_FILE_STAGE3="$OUT_S3" \
	]
	eval $vsim_args

	run -all
	quit -sim

}

puts	"=== run_all.do: done (processed $total / requested $NSETS) ==="
