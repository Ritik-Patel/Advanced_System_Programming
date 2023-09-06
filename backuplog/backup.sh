#!/bin/bash

#Define the user's home directory
userhmDirec="/home/patel6z6"
mkdir -p "$userhmDirec"
#Define the location where you want to store the backup files
userDefinedBackUpDirec="$userhmDirec/backup"
#Define and make a directory to store tar files of incremental backups
userDefinedIncBackUpDirec="$userDefinedBackUpDirec/ib"
mkdir -p "$userDefinedIncBackUpDirec"
#Define and make a directory to store tar files of complete backups
userDefinedFinalBackUpDirec="$userDefinedBackUpDirec/cb"
mkdir -p "$userDefinedFinalBackUpDirec"
#Define a file to keep a log of backups done
dirForBackUpsLog="$userDefinedBackUpDirec/backup.log"
touch "$dirForBackUpsLog"

#Define timestamps and time between 2 backup checks
longTimeStampForrmat="%a %d %b %Y %I:%M:%S %p %Z"
timeSmpForBackUp="%Y-%m-%d %H:%M:%S %Z"
hibernatingTime=120

#Define the prefixes for incremental backup and complete backup
finalBackUpFilePref=20001
increBackUpFilePref=10001

#Define a function to ccheck for complete bacck up stage
function fullBeck() {
  find "$userhmDirec" -name "*.txt" -type f -print0 2>/dev/null | tar czf "$userDefinedFinalBackUpDirec/cb$finalBackUpFilePref.tar" --null -T - >/dev/null 2>&1
  #If text files are found then create a complete backup
  if [ $? -eq 0 ]; then
    echo "$1  cb$finalBackUpFilePref.tar was created" >> "$dirForBackUpsLog"
    finalBackUpFilePref=$((finalBackUpFilePref + 1))
  #Prompt user that there are no text files in their system
  else
    echo "$1  No .txt files found for complete backup" >> "$dirForBackUpsLog"
  fi
}

#Define a function to ccheckk for incremental backup stage, if the system needs to take an inc backup or not
function possibleIncBeck() {
  new_files=$(find "$userhmDirec" -type f -name "*.txt" -newermt "$2" 2>/dev/null)
  #If new text files or modified tex files are found then create an inc backup
  if [ -n "$new_files" ]; then
    backup_file="ib$increBackUpFilePref.tar"
    tar -cf "$userDefinedIncBackUpDirec/$backup_file" $new_files >/dev/null 2>&1
    echo "$1  $backup_file was created" >> "$dirForBackUpsLog"
    increBackUpFilePref=$((increBackUpFilePref + 1))
  #Else prompt user that no changes in txt files, so no inc backup needed
  else
    echo "$1  No changes-Incremental backup was not created" >> "$dirForBackUpsLog"
  fi
}

#Define a function to intiate coomplete backup
function intiateFullBeck() {
  #Define the log time format
  timeCurLoc=$(TZ=America/New_York date +"$timeSmpForBackUp")
  timeCurLocLogForm=$(TZ=America/New_York date +"$longTimeStampForrmat")
  fullBeck "$timeCurLocLogForm"
}

#Define a function to intiate incremental backup
function initiateIncBeck() {
  timeCurLoc=$(TZ=America/New_York date +"$timeSmpForBackUp")
  for i in 1 2 3; do
    #Define the log time format
    timeCurLocLogForm=$(TZ=America/New_York date +"$longTimeStampForrmat")
    possibleIncBeck "$timeCurLocLogForm" "$timeCurLoc"
    timeCurLoc=$(date +"$timeSmpForBackUp")
    sleep $hibernatingTime
  done
}

#Run an inifinte loop in background to check for incremental and complete backup
while :; do
  intiateFullBeck
  sleep $hibernatingTime
  initiateIncBeck
done &
