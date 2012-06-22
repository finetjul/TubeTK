#! /bin/sh

MachineName=Dash21_Kitware

if [ $# -eq 0 ] || [ "$1" != "NoUpdate" ]; then
  echo "Updating"
  rm -rf /home/kitware/Dashboards/TubeTK-Debug

  # Update Dashboard repository
  cd /home/kitware/Dashboards/TubeTK_Dashboards
  git pull -f

  # Get new nightly script
  cp -f ${MachineName}_TubeTK_Nightly.sh ..
  cd ..
  chmod +x ${MachineName}_TubeTK_Nightly.sh
  
  # Run new nightly script without updating again
  ./${MachineName}_TubeTK_Nightly.sh NoUpdate
fi

# Run the nightly
/usr/local/bin/ctest -S TubeTK_Dashboards/${MachineName}_TubeTK_Nightly.cmake -D Nightly -V -VV -O ${machineName}_TubeTK_Nightly.log
