    set xlabel "M (Offered Load)"
    set autoscale
    set terminal png
    set ylabel "Delay"
    set title "Delay vs M"
    set style data yerrorlines
    set bars large
    do for [i=0:10] {
        fifo = sprintf("results/delay/mean_%d_delay_fifo.dat",i)
        rr = sprintf("results/delay/mean_%d_delay_rr.dat", i)
        drr = sprintf("results/delay/mean_%d_delay_drr.dat", i)
        outfile = sprintf("%d_delay.png",i)
        set output outfile
        plot fifo title 'FIFO', \
             rr  title 'RR', \
             drr  title 'DRR',
    }

    set ylabel "Delta Loads"
    set title "Delta vs M"
    set style data yerrorlines
    do for [i=0:10] {
        fifo = sprintf("results/tput/mean_%d_tput_fifo.dat",i)
        rr = sprintf("results/tput/mean_%d_tput_rr.dat", i)
        drr = sprintf("results/tput/mean_%d_tput_drr.dat", i)
        outfile = sprintf("%d_tput.png",i)
        set output outfile
        plot fifo using 1:2 title 'FIFO', \
             rr using 1:2 title 'RR', \
             drr using 1:2 title 'DRR'
    }

