echo "----------------------------------";
echo "BSEARCH";
echo "----------------------------------";

for i in 10 100 1000 10000 100000 1000000 10000000 100000000
do

    for j in 20 50 100 500
    do

        gcc bsearch.c stringsLib.c -DVM_NAME_SIZE=$j -O3  -o bsearch 
        gcc hash.c stringsLib.c -DMURMUR3 -DVM_NAME_SIZE=$j -O3  -o hashMURMUR3 
        gcc hash.c stringsLib.c -DDJB2 -DVM_NAME_SIZE=$j -O3  -o hashDJB2
        gcc hash.c stringsLib.c -DSDBM -DVM_NAME_SIZE=$j -O3  -o hashSDBM
        gcc hash.c stringsLib.c -DLOSELOSE -DVM_NAME_SIZE=$j -O3  -o hashLOSELOSE

        echo "----------------------------------";
        echo "      $i - $j   ";
        echo "----------------------------------";
        ./bsearch 100 $i;
        ./hashMURMUR3 100 $i;
        ./hashSDBM 100 $i;
        ./hashSDBM 100 $i;
        ./hashLOSELOSE 100 $i;
    done

done


rm bsearch hashMURMUR3 hashDJB2 hashSDBM hashLOSELOSE



