#include "PlaneItems.h"
#include <QJsonArray>
#include <QDebug>

#include "CEManeuvers.h"

BaseItem::~BaseItem()
{
    qDeleteAll(children);
}

QVariant BaseItem::data(int column) const
{
    return column_data.value(column);
}

void BaseItem::setData(int column, QVariant data)
{
    column_data[column] = data;
}

PlaneItem::PlaneItem(QJsonObject plane, BaseItem *parent) : BaseItem(Plane_Item_Type, parent)
{
    setData(Plane_Name,        plane.value("name"));
    setData(Plane_Era,         plane.value("plane_era"));
    setData(Fuel,              plane.value("fuel"));
    setData(Engine_HP,         plane.value("engine_hp"));
    setData(Engine_Critical,   plane.value("engine_critical"));
    setData(Wing_HP,           plane.value("wing_hp"));
    setData(Wing_Critical,     plane.value("wing_critical"));
    setData(Fuselage_HP,       plane.value("fuselage_hp"));
    setData(Fuselage_Critical, plane.value("fuselage_critical"));
    setData(Tail_HP,           plane.value("tail_hp"));
    setData(Tail_Critical,     plane.value("tail_critical"));
    setData(Rated_Dive,        plane.value("rated_dive"));
    setData(Rated_Climb,       plane.value("rated_climb"));
    setData(Max_Altitude,      plane.value("max_alt"));
    setData(Stability,         plane.value("stability"));
    setData(Is_Gliding,        false);
    setData(Is_On_Fire,        false);

    QJsonArray maneuvers = plane.value("maneuvers").toArray();
    for (int i=0; i<maneuvers.size(); ++i) {
        addChild(new ManeuverItem(maneuvers.at(i).toObject(), this));
    }

    QJsonArray crew_members = plane.value("crew").toArray();
    for (int i=0; i<crew_members.size(); ++i) {
        addChild(new CrewItem(crew_members.at(i).toObject(), this));
    }
}

PlaneItem::PlaneItem(BaseItem *parent) : BaseItem(Plane_Item_Type, parent)
{
    for (auto maneuver : master_maneuver_list) {
        addChild(new ManeuverItem(maneuver, this));
    }
}

QJsonObject PlaneItem::toJSON() const
{
    QJsonObject plane;
    plane["name"] = data(Plane_Name).toString();
    plane["plane_era"] = data(Plane_Era).toString();
    plane["fuel"] = data(Fuel).toInt();
    plane["engine_hp"] = data(Engine_HP).toInt();
    plane["engine_critical"] = data(Engine_Critical).toInt();
    plane["wing_hp"] = data(Wing_HP).toInt();
    plane["wing_critical"] = data(Wing_Critical).toInt();
    plane["fuselage_hp"] = data(Fuselage_HP).toInt();
    plane["fuselage_critical"] = data(Fuselage_Critical).toInt();
    plane["tail_hp"] = data(Tail_HP).toInt();
    plane["tail_critical"] = data(Tail_Critical).toInt();
    plane["rated_dive"] = data(Rated_Dive).toInt();
    plane["rated_climb"] = data(Rated_Climb).toInt();
    plane["max_alt"] = data(Max_Altitude).toString();
    plane["stability"] = data(Stability).toString();

    QJsonArray maneuvers;
    QJsonArray crew;
    for (int i=0; i<childCount(); ++i) {
        if (childAt(i)->getType() == Maneuver_Item_Type && childAt(i)->data(ManeuverItem::Added_To_Schedule).toBool()) {
            maneuvers << static_cast<ManeuverItem*>(childAt(i))->toJSON();
        }
        else if (childAt(i)->getType() == Crew_Item_Type) {
            crew << static_cast<CrewItem*>(childAt(i))->toJSON();
        }
    }

    plane["maneuvers"] = maneuvers;
    plane["crew"] = crew;
    return plane;
}

ManeuverItem::ManeuverItem(QJsonObject maneuver, BaseItem *parent) : BaseItem(Maneuver_Item_Type, parent)
{
    QString name = maneuver.value("name").toString();
    setData(Maneuver_Name,        name);
    setData(IsEnabled,            true); // By default all available maneuvers are enabled
    setData(Can_Be_Used,          true); // This needs to be set dynamically turn by turn
    setData(Speed,                name.right(1).toInt());
    setData(Direction,            name.right(2).left(1));
    QStringList tolerances =      maneuver.value("tolerances").toString().split("/");
    setData(Climb_Value,          tolerances.at(0));
    setData(Level_Value,          tolerances.at(1));
    setData(Dive_Value,           tolerances.at(2));
    setData(Can_Be_Repeated,      maneuver.value("can_be_repeated"));
    setData(Can_Reload,           maneuver.value("can_reload"));
    setData(Can_Put_Out_Fires,    maneuver.value("can_put_out_fires"));
    setData(Observer_Can_Reload,  maneuver.value("observer_can_reload"));
    setData(Is_Restricted,        maneuver.value("is_restricted"));
    setData(Is_Weight_Restricted, maneuver.value("is_weight_restricted"));
    setData(Is_Climb_Restricted,  maneuver.value("is_climb_restricted"));
    setData(Causes_Spin_Check,    maneuver.value("causes_spin_check"));
}

ManeuverItem::ManeuverItem(Maneuver maneuver, BaseItem *parent) : BaseItem(Maneuver_Item_Type, parent)
{
    setData(Maneuver_Name,       maneuver.name);
    setData(Speed,               maneuver.name.right(1).toInt());
    setData(Direction,           maneuver.name.right(2).left(1));
    setData(IsEnabled,           true);
    setData(Can_Be_Repeated,     true);
    setData(Added_To_Schedule,       false); // Maneuvers need to be added to the schedule to be saved
    setData(Climb_Value,         maneuver.climb_value);
    setData(Level_Value,         maneuver.level_value);
    setData(Dive_Value,          maneuver.dive_value);
    setData(Is_Restricted,       maneuver.is_restricted);
    setData(Is_Climb_Restricted, maneuver.is_climb_restricted);
    setData(Causes_Spin_Check,   maneuver.causes_spin_check);
}

QJsonObject ManeuverItem::toJSON() const
{
    QJsonObject maneuver;
    QStringList tolerances = {data(Climb_Value).toString(), data(Level_Value).toString(), data(Dive_Value).toString()};
    maneuver["name"] =                 data(Maneuver_Name).toString();
    maneuver["tolerances"] =           tolerances.join("/");
    maneuver["can_be_repeated"] =      data(Can_Be_Repeated).toBool();
    maneuver["can_reload"] =           data(Can_Reload).toBool();
    maneuver["can_put_out_fires"] =    data(Can_Put_Out_Fires).toBool();
    maneuver["observer_can_reload"] =  data(Observer_Can_Reload).toBool();
    maneuver["is_restricted"] =        data(Is_Restricted).toBool();
    maneuver["is_weight_restricted"] = data(Is_Weight_Restricted).toBool();
    maneuver["is_climb_restricted"] =  data(Is_Climb_Restricted).toBool();
    maneuver["causes_spin_check"] =    data(Causes_Spin_Check).toBool();
    return maneuver;
}

CrewItem::CrewItem(QJsonObject crew, BaseItem *parent) : BaseItem(Crew_Item_Type, parent)
{
    setData(Crew_Role, crew.value("role").toString());
    setData(Wounds, 0);
}

CrewItem::CrewItem(BaseItem *parent) : BaseItem(BaseItem::Crew_Item_Type, parent) {}

QJsonObject CrewItem::toJSON() const
{
    QJsonObject crew;
    crew["role"] = data(Crew_Role).toString();
    if (childCount() > 0) {
        crew["gun"] = static_cast<GunItem*>(childAt(0))->toJSON();
    }
    return crew;
}

GunItem::GunItem(QJsonObject gun, BaseItem *parent) : BaseItem(Gun_Item_Type, parent)
{
    setData(Ammo_Box_Capacity, gun.value("ammo_box_capacity"));
    setData(Ammo_Box_Count,    gun.value("ammo_box_count"));
    setData(Fire_Template,     gun.value("fire_template").toInt());
    setData(Fire_Base_3,       gun.value("fire_base_3").toInt());
    setData(Fire_Base_2,       gun.value("fire_base_2").toInt());
    setData(Fire_Base_1,       gun.value("fire_base_1").toInt());
    setData(Fire_Base_0,       gun.value("fire_base_0").toInt());
    setData(Gun_Destroyed,     false);
}

GunItem::GunItem(BaseItem *parent) : BaseItem(BaseItem::Gun_Item_Type, parent) {}

QJsonObject GunItem::toJSON() const
{
    QJsonObject gun;
    gun["name"] = data(Gun_Name).toString();
    gun["fire_template"]     = data(Fire_Template).toInt();
    gun["fire_base_3"]       = data(Fire_Base_3).toInt();
    gun["fire_base_2"]       = data(Fire_Base_2).toInt();
    gun["fire_base_1"]       = data(Fire_Base_1).toInt();
    gun["fire_base_0"]       = data(Fire_Base_0).toInt();
    gun["ammo_box_capacity"] = data(Ammo_Box_Capacity).toInt();
    gun["ammo_box_count"]    = data(Ammo_Box_Count).toInt();
    return gun;
}
