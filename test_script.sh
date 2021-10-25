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
# TODO: add all concurrent key words to searches
find_concurrent(){
    for fn in edu/cs300/*.java; do 
    echo "SEARCHING IN: $fn"
    grep -n "Thread" "$fn"
    echo
    done

    for fn in *.c; do 
    echo "SEARCHING IN: $fn"
    grep -n "Mutex" "$fn"
    echo
    done
}

find_concurrent