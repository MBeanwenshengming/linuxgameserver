create database account default character set utf8  collate utf8_general_ci;
use account;
create table tb_account (id bigint not null auto_increment primary key, accountname varchar(30) not null);
create index index_account_accountname on tb_account(accountname);

delimiter //

create procedure addaccount (accname varchar(20), out accid bigint)
begin
if not exists (select * from tb_account where accountname=accname) then
	insert into tb_account (accountname) values(accname);
end if;
select id into accid from tb_account where accountname=accname;
end
//

create procedure getallaccount()
begin	
	select * from tb_account;
end
//

create procedure delaccount(accname varchar(20))
begin
delete from tb_account where accountname=accname;
end
//

delimiter ;

create database gameworld default character set utf8  collate utf8_general_ci;
use gameworld;
create table tb_role (id bigint not null, roleid int not null, rolename varchar(20) not null, classid int not null);
create index index_role_id_roleid on tb_role (id, roleid);
create index index_role_name on tb_role (rolename);

#角色下一个id 
create table tb_roleidgenerate (roleid int not null);

delimiter //

create procedure getnextroleid(out roleidvalue int)
begin
	declare nowroleid int;
	select roleid into nowroleid from tb_roleidgenerate;
	set roleidvalue = nowroleid + 1;
	update tb_roleidgenerate set roleid=roleidvalue;
end
//

create procedure addrole (accountid bigint, roleidvalue int, rolenamevalue varchar(20), classidvalue int, out addresult int)
begin
	if exists(select * from tb_role where rolename=rolenamevalue) then
	begin
		set addresult = -1;		
	end;
	else
	begin
		insert into tb_role values(accountid, roleidvalue, rolenamevalue,classidvalue);
		set addresult = 0;
	end;		
	end if;
end
//

create procedure getroleinfo(accountidvalue bigint, roleidvalue int)
begin
	select * from tb_role where id = accountidvalue and roleid=roleidvalue;
end
//


create procedure getroleinfobyname(charname varchar(20))
begin
	select * from tb_role where rolename=charname;
end
//

create procedure delrole(accountidvalue bigint, roleidvalue int)
begin
	delete from tb_role where id=accountidvalue and roleid=roleidvalue;
end
//

create procedure delrolebyname(charname varchar(20))
begin
	delete from tb_role where rolename=charname;
end
//



delimiter ;

create table tb_item (itemno bigint auto_increment not null primary key, roleid int not null, itemid int not null, optid int not null default 0, optvalue int not null default 0 );
create index index_item_itemno on tb_item(itemno);
create index index_item_roleid on tb_item(roleid);

delimiter //

create procedure multirecordtest()
begin
	select * from tb_role;
	select * from tb_item;
end
//

create procedure multirecordandouttest(out charname varchar(20))
begin
	select * from tb_role;
	select * from tb_item;
	set charname = 'test you go';
end
//


create procedure additem(roleidvalue int, itemidvalue int, optidvalue int, optvaluevalue int, out itemnovalue bigint)
begin
	insert into tb_item (roleid, itemid, optid, optvalue) values(roleidvalue,itemidvalue,optidvalue,optvaluevalue);
	select @@IDENTITY into itemnovalue;
end
//

delimiter ;
