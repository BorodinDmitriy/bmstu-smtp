for i in {1..5}
do
	fileInTmp="./maildir/$i/Maildir/tmp/mail.eml"
	if [ -f "$fileInTmp" ]
	then
		mv "$fileInTmp" "./maildir/$i/Maildir/new/mail.eml"
		echo "$fileInTmp moved in new"
	fi

	fileInCurrent="./maildir/$i/Maildir/current/mail.eml"
	if [ -f "$fileInCurrent" ]
	then
		mv "$fileInCurrent" "./maildir/$i/Maildir/new/mail.eml"
		echo "$fileInCurrent moved in new"
	fi
done
