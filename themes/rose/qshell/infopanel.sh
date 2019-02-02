#!/bin/bash
source /etc/os-release
pkg_last_update=$((($(date +%s) - $(date -d $(sed -n '/upgrade$/x;${x;s/.\([0-9-]*\).*/\1/p}' /var/log/pacman.log) +%s)) / 86400))

echo "<style>
div {
}
.title {background:rgba(0,0,0,20%);}
.body {margin-left: 10px;}
th {text-align:left; font-weight:bold;}
hr { background-color:red;}
</style>
"
echo "<div>"

echo "
<table align='center' cellpadding='8' style='margin-bottom:30px;'>
    <tr>
        <td align='center'><img src='/home/user/.config/qshell/akari.png'></td>
    </tr>
    <tr>
        <td align='center'><h1>$(whoami)@$(cat /etc/hostname)</h1></td>
    </tr>
</table>
"

echo "
    <table style='background:rgba(0,0,0,20%);margin-bottom:20px;' width='100%' cellpadding='10'>
        <tr><td><h2>Packages</h2></td></tr>
    </table>
    <div class='body'>
        <table align='center' cellpadding='8'>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/devices/16/computer.svg' /></td>
                <th width='80%'>Distro</th>
                <td align='right'>$PRETTY_NAME</td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/status/22/install.svg' width='16' /></td>
                <th width='80%'>Installed</th>
                <td align='right'>$(pacman -Qq --color never|wc -l)</td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/actions/16/view-calendar.svg' /></td>
                <th width='80%'>Last updated</th>
                <td align='right'>$pkg_last_update days ago</td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/actions/16/help-about.svg' /></td>
                <th width='80%'>Kernel version</th>
                <td align='right'>$(uname -r)</td>
            </tr>
        </table>
    </div>
"
echo "
    <table style='background:rgba(0,0,0,20%);margin:20px 0;' width='100%' cellpadding='10'>
        <tr><td><h2>Mount points</h2></td></tr>
    </table>
    <div class='body'>
        <table align='center' cellpadding='8'>
            $(findmnt -rno target,used,size -t btrfs,ext4 | while read line; do
                folder=$(echo $line | cut -d' ' -f1)
                echo "<tr>
                    <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/devices/16/drive-harddisk.svg' /></td>
                    <th width='80%'>${folder/#$HOME/\~}</th>
                    <td align='right'>$(echo $line | cut -d' ' -f2)/$(echo $line | cut -d' ' -f3)</td>
                </tr>"
            done)
        </table>
    </div>
"

echo "</div>"
