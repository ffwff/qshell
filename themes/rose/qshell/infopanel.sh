#!/bin/bash
source /etc/os-release

output() {

pkg_last_update=$((($(date +%s) - $(date -d $(sed -n '/upgrade$/x;${x;s/.\([0-9-]*\).*/\1/p}' /var/log/pacman.log) +%s)) / 86400))
cpu_usage="$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1"%"}')"
if [[ "$cpu_usage" = "0%" ]]; then
    cpu_usage="1px"
fi
mem_usage="$(free -m | awk 'NR==2{printf "%.2f%%", $3*100/$2 }')"

string="<style>
div {
}
.title {background:rgba(0,0,0,20%);}
.body {margin-left: 10px;}
th {text-align:left; font-weight:bold;}
hr { background-color:red;}
</style>

<div>

    <table align='center' cellpadding='8' style='margin-bottom:10px;'>
        <tr>
            <td align='center'><img src='/home/user/.config/qshell/shiina.png'></td>
        </tr>
        <tr>
            <td align='center'><h1>$(whoami)@$(cat /etc/hostname)</h1></td>
        </tr>
    </table>

    <div class='body'>
        <table align='center' cellpadding='5' style='margin-bottom:15px'>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/Paper/22x22/panel/cpu-frequency-indicator.svg' width='16' /></td>
                <th width='165' valign='middle'>CPU</th>
                <td align='right' width='165' valign='middle'>
                    <table style='background: rgba(0,0,0,0.3);' width='100%'>
                        <tr>
                        <td style='background: #4a7fbd; text-align:center;' width='$cpu_usage'></td>
                        <td></td>
                        </tr>
                    </table>
                </td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/devices/16/computer.svg' width='16' /></td>
                <th width='165' valign='middle'>Memory</th>
                <td align='right' width='165' valign='middle'>
                    <table style='background: rgba(0,0,0,0.3);' width='100%'>
                        <tr>
                        <td style='background: #4a7fbd; text-align:center;' width='$mem_usage'></td>
                        <td></td>
                        </tr>
                    </table>
                </td>
            </tr>
        </table>
    </div>

    <table style='background:rgba(0,0,0,20%);margin-bottom:20px;' width='100%' cellpadding='10'>
        <tr><td><h2>Packages</h2></td></tr>
    </table>
    <div class='body'>
        <table align='center' cellpadding='8'>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/devices/16/computer.svg' /></td>
                <th width='165'>Distro</th>
                <td align='right' width='165'>$PRETTY_NAME</td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/status/22/install.svg' width='16' /></td>
                <th width='165'>Installed</th>
                <td align='right' width='165'>$(pacman -Qq --color never|wc -l)</td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/actions/16/view-calendar.svg' /></td>
                <th width='165'>Last updated</th>
                <td align='right' width='165'>$pkg_last_update days ago</td>
            </tr>
            <tr>
                <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/actions/16/help-about.svg' /></td>
                <th width='165'>Kernel version</th>
                <td align='right' width='165'>$(uname -r)</td>
            </tr>
        </table>
    </div>

    <table style='background:rgba(0,0,0,20%);margin:20px 0;' width='100%' cellpadding='10'>
        <tr><td><h2>Mount points</h2></td></tr>
    </table>
    <div class='body'>
        <table align='center' cellpadding='8'>
            $(findmnt -rno target,used,size -t btrfs,ext4 | while read line; do
                folder=$(echo $line | cut -d' ' -f1)
                echo "<tr>
                    <td valign='middle'><img src='file:///usr/share/icons/breeze-dark/devices/16/drive-harddisk.svg' /></td>
                    <th width='165'>${folder/#$HOME/\~}</th>
                    <td align='right' width='165'>$(echo $line | cut -d' ' -f2)/$(echo $line | cut -d' ' -f3)</td>
                </tr>"
            done)
        </table>
    </div>
</div>"

echo "$string"
>&2 echo -n "[flush]"

}

while true; do
    output;
    sleep 0.1s;
done
