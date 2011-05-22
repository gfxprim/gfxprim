#
# Takes directory as parameter
#
spinlock()
{
	I=0
	#echo -n "Trying to acquire lock in '$1' ."
	while ! mkdir "$1/.lock" &> /dev/null; do
		sleep 1;
		((I=I+1))
		if [ $I -gt 10 ]; then
			echo "Failed to acquire lock '`pwd`/.lock'"
			exit 1
		fi
	#	echo -n .
	done
	#echo " done"
}

spinunlock()
{
	#echo "Removing lock in '$1'"
	rmdir "$1/.lock" &> /dev/null
}
