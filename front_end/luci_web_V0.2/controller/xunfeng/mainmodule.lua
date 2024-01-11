module("luci.controller.xunfeng.mainmodule", package.seeall)

function index()
	local LoginPage = entry({"xunfeng"}, alias("xunfeng", "home"), translate("index"), 1)
	LoginPage.sysauth = 'root'
	LoginPage.sysauth_authenticator = "htmlauth"
	LoginPage.index = true
	entry({"login"}, alias("xunfeng", "home"))
	entry({"index"}, alias("xunfeng", "home"))

	entry({"xunfeng", "tophtm"}, template("xunfeng/top")).sysauth = false
	entry({"xunfeng", "lefthtm"}, template("xunfeng/left")).sysauth = false
	entry({"xunfeng", "bottomhtm"}, template("xunfeng/bottom")).sysauth = false

	entry({"xunfeng", "home"}, template("xunfeng/home"), translate("home"), 1)
	entry({"xunfeng", "network"}, template("xunfeng/network"), translate("network"), 2)
	entry({"xunfeng", "system"}, template("xunfeng/system"), translate("system"), 3)
	entry({"xunfeng", "user"}, template("xunfeng/user"), translate("user"), 4)
	entry({"xunfeng", "serial ports"}, template("xunfeng/serial ports"), translate("serial ports"), 5)
	entry({"xunfeng", "view"}, template("xunfeng/view"), translate("view"), 6)
	entry({"xunfeng", "backup"}, template("xunfeng/backup"), translate("bakcup"), 7)
	entry({"xunfeng", "update"}, template("xunfeng/update"), translate("update"), 8)
	entry({"xunfeng", "system information"}, template("xunfeng/system information"), translate("system information"), 9)
	entry({"xunfeng", "system tools"}, template("xunfeng/system tools"), translate("system tools"), 10)
	entry({"xunfeng", "system log"}, template("xunfeng/system log"), translate("system log"), 11)
	entry({"xunfeng", "reboot"}, template("xunfeng/reboot"), translate("reboot"), 12)

	entry({"xunfeng", "doLogout"}, call("Action_logout"))
	entry({"xunfeng", "doReboot"}, call("Action_reboot"))
	entry({"xunfeng", "doConfigNet"}, call("Action_confignet"))
	entry({"xunfeng", "doConfigSys"}, call("Action_configsys"))
	entry({"xunfeng", "doConfigUser"}, call("Action_configuser"))
	entry({"xunfeng", "doBackup"}, call("Action_backup"))
	entry({"xunfeng", "doRestore"}, call("Action_restore"))
	entry({"xunfeng", "doLog"}, call("Action_log"))
	entry({"xunfeng", "doPing"}, call("Action_ping"))
	entry({"xunfeng", "doUpdate"}, call("Action_update"))
	entry({"xunfeng", "doSaveUart"}, call("Action_saveuart"))
	-- entry({"xunfeng", "getSysInfo"}, call("Action_getsysinfo"))
	-- entry({"xunfeng", "getUSBLog"}, call("Action_getusblog"))
	
end

function Action_logout()
	local dsp = require "luci.dispatcher"
	local utl = require "luci.util"
	local sid = dsp.context.authsession

	if sid then
		utl.ubus("session", "destroy", { ubus_rpc_session = sid })
		if luci.http.getenv('HTTPS') == 'on' then
			luci.http.header("Set-Cookie", "sysauth_https=; expires=Thu, 01 Jan 1970 01:00:00 GMT; path=%s" % dsp.build_url())
		end
		luci.http.header("Set-Cookie", "sysauth_http=; expires=Thu, 01 Jan 1970 01:00:00 GMT; path=%s" % dsp.build_url())
	end
	luci.template.render("xunfeng/logout")
end

function Action_confignet()
	luci.http.write("<html>"..
						"<header>"..
							"<title>网络配置结果</title>"..
						"</header>"..
						"<body>"..
							"<h3>配置成功，目前仅测试调用，没有具体实现</h3>"..
							"<a href='network' target=_parent>返回</a>"..
						"</body>"..
					"</html>"
				)
end

function Action_configsys()
	local http = require("luci.http")
	local sys = require("luci.sys")

	local description = http.formvalue("description")
	local contact = http.formvalue("contact")
	local location = http.formvalue("location")

	if(description and contact and location)
	then
		sys.exec("uci set mss_sysconfig.summary=interface")
		sys.exec("uci set mss_sysconfig.summary.description="..description)
		sys.exec("uci set mss_sysconfig.summary.contact="..contact)
		sys.exec("uci set mss_sysconfig.summary.location="..location)
		sys.exec("uci commit mss_sysconfig")
		http.write_json({"res", "success", "des", description, "contact", contact, "location", location})
	else
		http.redirect("system")
	end
end

function Action_configuser()
	luci.http.write("<html>"..
						"<header>"..
							"<title>用户信息配置结果</title>"..
						"</header>"..
						"<body>"..
							"<h3>配置成功，目前仅测试调用，没有具体实现</h3>"..
							"<a href='user' target=_parent>返回</a>"..
						"</body>"..
					"</html>"
				)
end

function Action_backup()
	local bakfilename = "ConfigBackup"..os.time(date)..".bak"
	local bakcmd = "tar zcf /tmp/".. bakfilename .." -C /etc/config ."
	luci.sys.exec(bakcmd)
	luci.http.write("<html>"..
						"<header>"..
							"<title>备份结果</title>"..
						"</header>"..
						"<body>"..
							"<h3>备份成功，已备份到/tmp/"..bakfilename.."</h3>"..
							"<a href='backup' target=_parent>返回</a>"..
						"</body>"..
					"</html>"
				)
end

function Action_restore()
	local restorebak = luci.http.formvalue("upload")
	local bakfilename = "ConfigBackup"..os.time(date)..".bak"
	local file = io.open("/tmp/"..bakfilename, "wb")
	file:write(unpack(restorebak))
	file:close()
	luci.http.write("<html>"..
						"<header>"..
							"<title>恢复结果</title>"..
						"</header>"..
						"<body>"..
							"<h3>恢复成功，重启后应用新配置，保存文件为"..bakfilename.."</h3>"..
							"<a href='backup' target=_parent>返回</a>"..
						"</body>"..
					"</html>"
				)
end

function Action_log()
	luci.http.write("<html>"..
						"<header>"..
							"<title>生成日志结果</title>"..
						"</header>"..
						"<body>"..
							"<h3>日志生成成功，目前仅测试调用，没有具体实现</h3>"..
							"<a href='system log' target=_parent>返回</a>"..
						"</body>"..
					"</html>"
				)
end

function Action_ping()
	local hostip = luci.http.formvalue("host")
	luci.http.write(luci.sys.exec("ping "..hostip.." -c 1"))
end

function Action_update()
	luci.http.write("<html>"..
						"<header>"..
							"<title>更新结果</title>"..
						"</header>"..
						"<body>"..
							"<h3>更新成功，目前仅测试调用，没有具体实现</h3>"..
							"<a href='update' target=_parent>返回</a>"..
						"</body>"..
					"</html>"
				)
end

function Action_reboot()
	local sys = require("luci.sys")
	sys.exec("/etc/init.d/network restart && rm -rf /tmp/luci-*")
	luci.http.redirect("home")
end

function Action_saveuart()
	local http = require("luci.http")
	luci.sys.exec(">/tmp/log1.log")
	for i=1, 16 do
		local speed = http.formvalue(string.format("speed%d" % i))
		local databit = http.formvalue(string.format("databit%d" % i))
		local parity = http.formvalue(string.format("parity%d" % i))
		local stopbit = http.formvalue(string.format("stopbit%d" % i))
		local flowcontrol = http.formvalue(string.format("flowcontrol%d" % i))
		local portmode = http.formvalue(string.format("portmode%d" % i))

		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config=%s" % {(i - 1), "interface"}))
		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config.speed=%s" % {(i - 1), speed}))
		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config.databit=%s" % {(i - 1), databit}))
		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config.parity=%s" % {(i - 1), parity}))
		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config.stopbit=%s" % {(i - 1), stopbit}))
		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config.flowcontrol=%s" % {(i - 1), flowcontrol}))
		luci.sys.exec("uci set "..string.format("serial_port_para_%d.config.portmode=%s" % {(i - 1), portmode}))
		luci.sys.exec("uci commit "..string.format("serial_port_para_%d" % (i - 1)))
	end
	
	http.write("ok")
end

function Action_getsysinfo()
	
end

function Action_getusblog()
	
end

return {
	index = index,
	Action_logout = Action_logout,
	Action_confignet = Action_confignet,
	Action_configsys = Action_configsys,
	Action_configuser = Action_configuser,
	Action_backup = Action_backup,
	Action_restore = Action_restore,
	Action_log = Action_log,
	Action_ping = Action_ping,
	Action_update = Action_update,
	Action_reboot = Action_reboot,
	Action_saveuart = Action_saveuart,
	Action_getsysinfo = Action_getsysinfo,
	Action_getusblog = Action_getusblog
}
