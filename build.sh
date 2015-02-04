#!/bin/bash

if [ $( lsmod | egrep -c "^rtl8821ae" ) -lt 1 ]; then
  sudo apt-get update
  sudo apt-get install -y linux-headers-$( uname -r )

  cd
  mkdir -p ~/src > /dev/null 2>&1
  cd ~/src

  git clone https://kernel.googlesource.com/pub/scm/linux/kernel/git/firmware/linux-firmware
  for fw in $( find linux-firmware -name rtl8821*.bin ); do
    if [ -r $fw ]; then
      sudo cp $fw /lib/firmware/rtlwifi/
      sudo cp $fw /lib/firmware/$( uname -r )/rtlwifi/
    fi
  done

  if [ ! -d rtl8821 ]; then
    git clone https://github.com/kevinmullican/rtl8821.git
    cd rtl8821 && git checkout master && cd ..
  fi
  pat=$( pwd )/$( find . -name "rtl8821*.patch" | head -1 )
  bpf=$( pwd )/$( find . -name "backport_funcs.h" | head -1 )
  if [ -r $pat -a -r $bpf ]; then
    git clone https://kernel.googlesource.com/pub/scm/linux/kernel/git/next/linux-next.git
    rtldir=$( find linux-next -type d -name rtlwifi )
    if [ -d $rtldir ]; then
      cd $rtldir
      rm backport_funcs.h *.o *.ko rtl8821ae/*.o rtl8821ae/*.ko btcoexist/*.o btcoexist/*.ko
      git reset --hard
      git apply $pat
      cp $bpf .
      make
      if [ $? -eq 0 ]; then
        rd=rtlwifi
        od=/lib/modules/$( uname -r )/kernel/drivers/net/wireless
        dd=/lib/modules/disabled/kernel/drivers/net/wireless
        sd=/lib/modules/$(uname -r)/kernel/drivers/staging

        if [ -d ${od}/$rd ]; then
          sudo rmmod btcoexist > /dev/null 2>&1
          sudo rmmod rtl_usb > /dev/null 2>&1
          sudo rmmod rtl_pci > /dev/null 2>&1
          sudo rmmod rtlwifi > /dev/null 2>&1
          sudo mkdir -p $dd > /dev/null 2>&1
          if [ -d $dd ]; then
            sudo mv ${od}/$rd $dd
          fi
        fi

        sudo mkdir -p ${sd}/$rd > /dev/null 2>&1
        if [ -d ${sd}/$rd ]; then
          for file in rtlwifi.ko rtl_pci.ko btcoexist/btcoexist.ko rtl8821ae/rtl8821ae.ko; do
            if [ -e $file ]; then
              sudo cp $file ${sd}/$rd
            fi
          done
          sudo depmod -a
          sudo modprobe rtl8821ae
        fi
      fi
    fi
  fi

  cd
fi

