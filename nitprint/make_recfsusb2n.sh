#!/bin/sh
./nitprint $1 5 | nkf -Z | sed -e 's/　/ /g' | sort > tmp.txt
echo ";Ch\tFreq\tSID\tTSID"
echo ";BS"
l=
while read c t f i d x s n; do
    if [ "$d" != "$l" ]; then
        a=`expr \( ${i} - 1049480 \) / 19180 + 101`
        echo "BS${c}_${t}\t${a}\t${s}\t${x}\t;${n}"
    fi
    l=${d}
done < tmp.txt
echo ""
echo ";CS"
echo "CS2\t202\t296\t0x6020"
echo "CS4\t204\t250\t0x7040"
echo "CS6\t206\t294\t0x7060"
echo "CS8\t208\t55\t0x6080"
echo "CS10\t210\t219\t0x60a0"
echo "CS12\t212\t254\t0x70c0"
echo "CS14\t214\t227\t0x70e0"
echo "CS16\t216\t290\t0x7100"
echo "CS18\t218\t240\t0x7120"
echo "CS20\t220\t307\t0x7140"
echo "CS22\t222\t161\t0x7160"
echo "CS24\t224\t223\t0x7180"
