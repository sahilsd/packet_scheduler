import collections
d = collections.defaultdict(list)
file_list = []
for i in range(0,11):
#    tput_fifo = str(i)+"_tput_fifo.dat"
#    delay_fifo = str(i)+"_delay_fifo.dat"
#    tput_rr = str(i)+"_tput_rr.dat"
#    tput_fifo = str(i)+"_tput_rr.dat"
#    tput_fifo = str(i)+"_tput_drr.dat"
#    tput_fifo = str(i)+"_delay_drr.dat"

#    tput_fifo = str(i)+"_delay_fifo.dat"
#    tput_fifo = str(i)+"_delay_rr.dat"
#    tput_drr = str(i)+"_tput_drr.dat"
#    delay_drr = str(i)+"_delay_drr.dat"

    #print tput_fifo, tput_rr, tput_drr, delay_fifo, delay_rr, delay_drr

    #for tput_fifo in xrange(str(i)+"_tput_fifo.dat", str(i)+"_delay_fifo.dat", str(i)+"_tput_rr.dat", str(i)+"_tput_rr.dat"):
    file_list.append(str(i)+"_tput_fifo.dat")
    file_list.append(str(i)+"_delay_fifo.dat")

    file_list.append(str(i)+"_tput_rr.dat")
    file_list.append(str(i)+"_delay_rr.dat")

    file_list.append(str(i)+"_tput_drr.dat")
    file_list.append(str(i)+"_delay_drr.dat")

    for file_idx in xrange(0,6):
        tput_fifo = file_list[file_idx]
        print tput_fifo
        tf = open(tput_fifo, 'r')
#     tf = open(tput_rr, 'r')
#    td = open(tput_drr, 'r')

#    df = open(delay_fifo, 'r')
#    dr = open(delay_rr, 'r')
#    dd = open(delay_drr, 'r')

        out_tf = open("mean_"+tput_fifo, "a+")
        for line in tf:
            line = line.strip()
            columns = line.split()
            #print "append", float(columns[0]), float(columns[1])
            d[float(columns[0])].append(float(columns[1]))
        for k,v in sorted(d.items()):
            mean = 0
            print k, ":"
            for i in range(0,5):
                print v[i]
                mean += v[i]
            mean = mean/5
            print k, "mean:", mean
            variance = 0
            for i in range(0,5):
                variance += pow(mean - v[i], 2);
            variance = variance/5
            ci = 1.645*pow((variance/5),0.5)
            print k, "variance:", variance, "90% CI: +/-", ci
            out_tf.write(str(k)+" \t"+str(mean)+" \t"+str(ci)+"\n");
        d.clear()
    tf.close()
    del file_list[:]
#df.close()
