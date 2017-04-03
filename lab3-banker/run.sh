inputFiles="./input/*"
for f in $inputFiles; do
    echo $f >> allOutputs
    ./a.out $f >> allOutputs
    echo $f "done"
done