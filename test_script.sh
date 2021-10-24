#! /bin/sh
# order of programs running should not matter
# build project then run the two exectutables.
test_project(){
    echo "BUILDING"
    make all
    echo "DONE BUILDING"
    # java -cp . -Djava.library.path=.edu.cs300.ReportingSystem # for server
    java edu.cs300.ReportingSystem # for local
    ./process_records < data
}

# search all files for any concurrent parts
# TODO: search through c files and find all concurrent keywords in java
find_concurrent(){
    for fn in edu/cs300/*.java; do 
    echo "SEARCHING IN: $fn"
    grep -n "Thread" "$fn"
    echo
    done
}

find_concurrent