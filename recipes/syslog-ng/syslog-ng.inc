DESCRIPTION = "Alternative system logger daemon"
DEPENDS = "libol flex eventlog glib-2.0"

# syslog initscript is handled explicitly because order of
# update-rc.d and update-alternatives is important
DEPENDS_append = " update-rc.d update-rc.d-native"
RDEPENDS_${PN}_append = " ${@base_conditional("ONLINE_PACKAGE_MANAGEMENT", "none", "", "update-rc.d", d)}"

INC_PR = "r4"

inherit autotools

EXTRA_OECONF = "--with-libol=${STAGING_BINDIR_CROSS}/ --enable-dynamic-linking"

do_install_append() {
        install -d ${D}/${sysconfdir}/${PN}
        install ${WORKDIR}/syslog-ng.conf ${D}${sysconfdir}/${PN}.conf
        install -d ${D}/${sysconfdir}/init.d
        install -m 755 ${WORKDIR}/initscript ${D}/${sysconfdir}/init.d/syslog.${PN}
}

CONFFILES_${PN} = "${sysconfdir}/${PN}.conf"

pkg_postinst_${PN} () {
        update-alternatives --install ${sysconfdir}/init.d/syslog syslog-init syslog.${PN} 200

        if test "x$D" != "x"; then
                OPT="-r $D"
        else
                OPT="-s"
        fi
        # remove all rc.d-links potentially created from alternative
        # syslog packages before creating new ones
        update-rc.d $OPT -f syslog remove
        update-rc.d $OPT syslog start 20 2 3 4 5 . stop 90 0 1 6 .
}

pkg_prerm_${PN} () {
        if test "x$D" = "x"; then
                if test "$1" = "upgrade" -o "$1" = "remove"; then
                        /etc/init.d/syslog stop
                fi
        fi

        update-alternatives --remove syslog-init syslog.${PN}
}

pkg_postrm_${PN} () {
        if test "x$D" != "x"; then
                OPT="-r $D"
        else
                OPT=""
        fi
        if test "$1" = "remove" -o "$1" = "purge"; then
                if ! test -e "/etc/init.d/syslog"; then
                        update-rc.d $OPT syslog remove
                fi
        fi
}