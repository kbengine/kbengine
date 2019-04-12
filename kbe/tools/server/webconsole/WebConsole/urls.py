"""KBEWebConsole URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/1.8/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  url(r'^$', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  url(r'^$', Home.as_view(), name='home')
Including another URLconf
    1. Add a URL to urlpatterns:  url(r'^blog/', include('blog.urls'))
"""
from django.conf.urls import include, url
from . import views, auth, views_console, views_profile, views_log, views_watcher, views_status, views_spaceviewer

urlpatterns = [
	url(r'^login$', auth.login, name = "login"),
	url(r'^logout$', auth.logout, name = "logout"),
	url(r'^user/manage$', auth.user_manage, name = "user_manage"),
	url(r'^user/add$', auth.user_add, name = "user_add"),
	url(r'^user/delete$', auth.user_delete, name = "user_delete"),
	url(r'^user/change_pwd$', auth.change_pwd, name = "change_user_pwd"),
	url(r'^user/change_user/(?P<userID>[0-9]+)$', auth.change_user, name = "change_user_user"),

	url(r'^$', views.index, name = "index"),
	url(r'^index$', views.index, name = "index"),

	url(r'^components/$', views.components_manage, name = "components_manage"),
	url(r'^components/manage$', views.components_manage, name = "components_manage"),
	url(r'^components/run$', views.components_run, name = "components_run"),
	url(r'^components/shutdown$', views.components_shutdown, name = "components_shutdown"),
	url(r'^components/(?P<ct>[0-9]+)/(?P<cid>[0-9]+)/shutdown$', views.components_stop, name = "components_stop"),
	url(r'^components/(?P<ct>[0-9]+)/(?P<cid>[0-9]+)/kill$', views.components_kill, name = "components_kill"),
	url(r'^components/(?P<ct>[0-9]+)/(?P<cid>[0-9]+)/query$', views.components_one_query, name = "components_one_query" ),
	# url(r'^components/(?P<ct>[0-9]+)$', views.components_group_query, name = "components_group_query" ),
	url(r'^components/query$', views.components_query, name = "components_query" ),
	url(r'^components/query_machines$', views.components_query_machines, name = "components_query_machines" ),
	url(r'^components/save_layout$', views.components_save_layout, name = "components_save_layout" ),
	url(r'^components/show_layout$', views.components_show_layout, name = "components_show_layout" ),
	url(r'^components/delete_layout$', views.components_delete_layout, name = "components_delete_layout" ),
	url(r'^components/load_layout$', views.components_load_layout, name = "components_load_layout" ),

	url(r'^machines/show_all$', views.machines_show_all, name = "machines_show_all" ),

	url(r'^console/show_components$', views_console.show_components, name = "console_show_components" ),
	url(r'^console/connect$', views_console.connect, name = "console_connect" ),
	url(r'^console/process_cmd$', views_console.process_cmd, name = "console_process_cmd" ),

	url(r'^profile/show_components$', views_profile.show_components, name = "console_show_components" ),
	url(r'^profile/connect$', views_profile.connect, name = "console_connect" ),
	url(r'^profile/process_cmd$', views_profile.process_cmd, name = "console_process_cmd" ),

	url(r'^log/connect',views_log.connect, name = "real_time_log" ),
	url(r'^log/process_cmd$', views_log.process_cmd, name = "log_process_cmd" ),
	url(r'^log/connect/pull',views_log.pull_log,name = "pull_log"),

	url(r'^watcher/show_components',views_watcher.show_components, name = "watcher_show_components" ),
	url(r'^watcher/connect',views_watcher.connect, name = "watcher_connect" ),
	url(r'^watcher/process_cmd$', views_watcher.process_cmd, name = "watcher_process_cmd" ),

	url(r'^status/show_components',views_status.show_components, name = "status_show_components" ),
	url(r'^status/connect',views_status.connect, name = "status_connect" ),
	url(r'^status/process_cmd$', views_status.process_cmd, name = "status_process_cmd" ),

	url(r'^spaceviewer/show',views_spaceviewer.show_components, name = "show_components" ),
	url(r'^spaceviewer/process_cmd$', views_spaceviewer.process_cmd, name = "spaceviewer_process_cmd" ),
	url(r'^spaceviewer/cell_process_cmd$', views_spaceviewer.cell_process_cmd, name = "spaceviewer_cell_process_cmd" ),
]
