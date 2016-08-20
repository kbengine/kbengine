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
from . import views, auth

urlpatterns = [
    url(r'^login$', auth.login, name = "login"),
    url(r'^logout$', auth.logout, name = "logout"),
    url(r'^user/manage$', auth.user_manage, name = "user_manage"),
    url(r'^user/add$', auth.user_add, name = "user_add"),
    url(r'^user/delete$', auth.user_delete, name = "user_delete"),
    url(r'^user/change_pwd$', auth.change_pwd, name = "change_user_pwd"),

    url(r'^$', views.index, name = "index"),
    url(r'^index$', views.index, name = "index"),
    
    url(r'^components/$', views.components_manage, name = "components_manage"),
    url(r'^components/manage$', views.components_manage, name = "components_manage"),
    url(r'^components/run$', views.components_run, name = "components_run"),
    url(r'^components/shutdown$', views.components_shutdown, name = "components_shutdown"),
    url(r'^components/query$', views.components_query, name = "components_query" ),
    url(r'^components/query_machines$', views.components_query_machines, name = "components_query_machines" ),
    url(r'^components/save_layout$', views.components_save_layout, name = "components_save_layout" ),
    url(r'^components/show_layout$', views.components_show_layout, name = "components_show_layout" ),
    url(r'^components/delete_layout$', views.components_delete_layout, name = "components_delete_layout" ),
    url(r'^components/load_layout$', views.components_load_layout, name = "components_load_layout" ),
   
    url(r'^machines/show_all$', views.machines_show_all, name = "machines_show_all" ),

    url(r'^console/show_components$', views.console_show_components, name = "console_show_components" ),
    url(r'^console/connect$', views.console_connect, name = "console_connect" ),
    url(r'^console/process_cmd$', views.console_process_cmd, name = "console_process_cmd" ),
   
]
