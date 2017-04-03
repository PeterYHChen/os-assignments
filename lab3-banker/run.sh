g++ lab3-banker.cpp

rm allOutputs

inputArr=(./input/*)
outputArr=(./output/*)

for ((i=0; i<${#inputArr[@]}; i++)); do
    echo "${inputArr[$i]}" >> allOutputs
    ./a.out ${inputArr[$i]} >> allOutputs
    echo "${inputArr[$i]}done"

    printf "${outputArr[$i]}\n\n" >> allOutputs
    cat ${outputArr[$i]} >> allOutputs
    printf "\n--------------------------------------------------------------------\n" >> allOutputs
    printf "${outputArr[$i]}done\n\n"
done