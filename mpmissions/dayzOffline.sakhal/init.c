void main()
{
	//INIT ECONOMY--------------------------------------
	Hive ce = CreateHive();
	if ( ce )
		ce.InitOffline();

	//DATE RESET AFTER ECONOMY INIT-------------------------
	int year, month, day, hour, minute;
	int reset_month = 2, reset_day = 1;
	GetGame().GetWorld().GetDate(year, month, day, hour, minute);

	if ((month == reset_month) && (day < reset_day))
	{
		GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
	}
	else
	{
		if ((month == reset_month + 1) && (day > reset_day))
		{
			GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
		}
		else
		{
			if ((month < reset_month) || (month > reset_month + 1))
			{
				GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
			}
		}
	}

	// 预加载/生成出生装备配置，避免必须等玩家出生才创建文件
	GetSurviveStartingLoadout();
}

// 与 Profiles/survive/ExpansionMod/Loadouts/*.json 的结构兼容，
// 目标：只改 $profile:StartingGearLoadout.json 即可调整出生装备
class SurviveLoadoutQuantity
{
	float Min;
	float Max;

	void SurviveLoadoutQuantity()
	{
		Min = 0.0;
		Max = 0.0;
	}
}

class SurviveLoadoutHealth
{
	float Min;
	float Max;
	string Zone;

	void SurviveLoadoutHealth()
	{
		Min = 0.0;
		Max = 0.0;
		Zone = "";
	}
}

class SurviveLoadoutAttachment
{
	string SlotName;
	ref array<ref SurviveLoadoutItem> Items;

	void SurviveLoadoutAttachment()
	{
		SlotName = "";
		Items = new array<ref SurviveLoadoutItem>;
	}
}

class SurviveLoadoutItem
{
	string ClassName;
	float Chance;
	ref SurviveLoadoutQuantity Quantity;
	ref array<ref SurviveLoadoutHealth> Health;
	ref array<ref SurviveLoadoutAttachment> InventoryAttachments;
	ref array<ref SurviveLoadoutItem> InventoryCargo;
	ref array<string> Sets;

	void SurviveLoadoutItem()
	{
		ClassName = "";
		Chance = 1.0;
		Quantity = new SurviveLoadoutQuantity();
		Health = new array<ref SurviveLoadoutHealth>;
		InventoryAttachments = new array<ref SurviveLoadoutAttachment>;
		InventoryCargo = new array<ref SurviveLoadoutItem>;
		Sets = new array<string>;
	}
}

class SurviveStartingLoadout: SurviveLoadoutItem
{
	void SurviveStartingLoadout()
	{
		// 基类 SurviveLoadoutItem() 构造会自动调用；这里不要手动调用。
	}
}

static ref SurviveStartingLoadout g_SurviveStartingLoadout;
static bool g_SurviveStartingLoadoutLoaded;

static float Clamp01(float v)
{
	if ( v < 0.0 ) return 0.0;
	if ( v > 1.0 ) return 1.0;
	return v;
}

static int GetSlotIdSafe(string slotName)
{
	if ( slotName == "" )
		return -1;
	return InventorySlots.GetSlotIdFromString( slotName );
}

static SurviveLoadoutItem PickWeightedItem(array<ref SurviveLoadoutItem> items)
{
	if ( !items || items.Count() == 0 )
		return null;

	float total = 0.0;
	for ( int i = 0; i < items.Count(); i++ )
	{
		SurviveLoadoutItem it = items[i];
		if ( it )
			total = total + Math.Max( 0.0, it.Chance );
	}

	if ( total <= 0.0 )
		return null;

	float r = Math.RandomFloat( 0.0, total );
	float acc = 0.0;
	for ( int j = 0; j < items.Count(); j++ )
	{
		SurviveLoadoutItem it2 = items[j];
		if ( !it2 )
			continue;
		acc = acc + Math.Max( 0.0, it2.Chance );
		if ( r <= acc )
			return it2;
	}

	return items[items.Count() - 1];
}

static void ApplyHealthFromDef(EntityAI itemEnt, array<ref SurviveLoadoutHealth> healthDefs)
{
	if ( !itemEnt || !healthDefs || healthDefs.Count() == 0 )
		return;

	for ( int i = 0; i < healthDefs.Count(); i++ )
	{
		SurviveLoadoutHealth h = healthDefs[i];
		if ( !h )
			continue;

		float minH = Clamp01( h.Min );
		float maxH = Clamp01( h.Max );
		float rndH = Math.RandomFloat( minH, maxH );

		if ( h.Zone != "" )
			itemEnt.SetHealth01( h.Zone, "", rndH );
		else
			itemEnt.SetHealth01( "", "", rndH );
	}
}

static void ApplyQuantityFromDef(EntityAI itemEnt, SurviveLoadoutQuantity qty)
{
	if ( !itemEnt || !qty )
		return;
	if ( qty.Min == 0.0 && qty.Max == 0.0 )
		return;

	ItemBase itemBase;
	if ( !Class.CastTo( itemBase, itemEnt ) )
		return;
	if ( !itemBase.HasQuantity() )
		return;

	float minQ = qty.Min;
	float maxQ = qty.Max;
	if ( maxQ < minQ )
	{
		float t = minQ;
		minQ = maxQ;
		maxQ = t;
	}

	float rnd = Math.RandomFloat( minQ, maxQ );
	int qMax = itemBase.GetQuantityMax();
	float target = rnd;
	if ( maxQ <= 1.0 && qMax > 0 )
		target = Math.Round( qMax * rnd );

	itemBase.SetQuantity( target );
}

static EntityAI CreateAttachmentSafe(EntityAI parent, string slotName, string className)
{
	if ( !parent || className == "" )
		return null;

	int slotId = GetSlotIdSafe( slotName );
	if ( slotId >= 0 )
		return parent.GetInventory().CreateAttachmentEx( className, slotId );

	return parent.GetInventory().CreateAttachment( className );
}

static EntityAI CreateInCargoSafe(EntityAI parent, string className)
{
	if ( !parent || className == "" )
		return null;
	return parent.GetInventory().CreateInInventory( className );
}

static void ApplyItemRecursive(EntityAI created, SurviveLoadoutItem def, EntityAI cargoReceiver = null)
{
	if ( !created || !def )
		return;

	ApplyHealthFromDef( created, def.Health );
	ApplyQuantityFromDef( created, def.Quantity );

	// 附件：每个 attachment 组（SlotName）从 Items 中选 1 个
	if ( def.InventoryAttachments )
	{
		for ( int a = 0; a < def.InventoryAttachments.Count(); a++ )
		{
			SurviveLoadoutAttachment attGroup = def.InventoryAttachments[a];
			if ( !attGroup || !attGroup.Items )
				continue;

			SurviveLoadoutItem picked = PickWeightedItem( attGroup.Items );
			if ( !picked )
				continue;
			if ( picked.ClassName == "" )
				continue;

			EntityAI att = CreateAttachmentSafe( created, attGroup.SlotName, picked.ClassName );
			ApplyItemRecursive( att, picked, cargoReceiver );
		}
	}

	// 货物：默认逐条按 Chance 判定生成；如果使用 Sets 分组，则每组只生成 1 个（按权重选）
	if ( def.InventoryCargo )
	{
		ref map<string, ref array<ref SurviveLoadoutItem>> grouped = new map<string, ref array<ref SurviveLoadoutItem>>;
		for ( int c = 0; c < def.InventoryCargo.Count(); c++ )
		{
			SurviveLoadoutItem cargoDef = def.InventoryCargo[c];
			if ( !cargoDef )
				continue;

			string setKey = "";
			if ( cargoDef.Sets && cargoDef.Sets.Count() > 0 )
				setKey = cargoDef.Sets[0];

			if ( setKey != "" )
			{
				if ( !grouped.Contains( setKey ) )
					grouped.Insert( setKey, new array<ref SurviveLoadoutItem> );
				grouped.Get( setKey ).Insert( cargoDef );
			}
			else
			{
				if ( Math.RandomFloat( 0.0, 1.0 ) <= Clamp01( cargoDef.Chance ) )
				{
					EntityAI cargo = CreateInCargoSafe( created, cargoDef.ClassName );
					if ( !cargo && cargoReceiver && cargoReceiver != created )
						cargo = CreateInCargoSafe( cargoReceiver, cargoDef.ClassName );
					ApplyItemRecursive( cargo, cargoDef, cargoReceiver );
				}
			}
		}

		for ( int g = 0; g < grouped.Count(); g++ )
		{
			array<ref SurviveLoadoutItem> candidates = grouped.GetElement( g );
			SurviveLoadoutItem chosen = PickWeightedItem( candidates );
			if ( chosen && chosen.ClassName != "" )
			{
				EntityAI cargo2 = CreateInCargoSafe( created, chosen.ClassName );
				if ( !cargo2 && cargoReceiver && cargoReceiver != created )
					cargo2 = CreateInCargoSafe( cargoReceiver, chosen.ClassName );
				ApplyItemRecursive( cargo2, chosen, cargoReceiver );
			}
		}
	}
}

static void ApplyStartingLoadoutToPlayer(PlayerBase player, SurviveStartingLoadout loadout)
{
	if ( !player || !loadout )
		return;

	if ( loadout.InventoryAttachments )
	{
		for ( int i = 0; i < loadout.InventoryAttachments.Count(); i++ )
		{
			SurviveLoadoutAttachment slotGroup = loadout.InventoryAttachments[i];
			if ( !slotGroup || !slotGroup.Items )
				continue;

			SurviveLoadoutItem picked = PickWeightedItem( slotGroup.Items );
			if ( !picked || picked.ClassName == "" )
				continue;

			// 清理该槽位已有物品（避免重复）
			if ( slotGroup.SlotName != "" )
			{
				EntityAI existing = player.FindAttachmentBySlotName( slotGroup.SlotName );
				if ( existing )
					GetGame().ObjectDelete( existing );
			}

			EntityAI created = CreateAttachmentSafe( player, slotGroup.SlotName, picked.ClassName );
			ApplyItemRecursive( created, picked, player );
		}
	}

	// 允许在 root 的 InventoryCargo 里直接塞进玩家背包/衣服可用空间
	if ( loadout.InventoryCargo )
	{
		for ( int c = 0; c < loadout.InventoryCargo.Count(); c++ )
		{
			SurviveLoadoutItem cargoDef = loadout.InventoryCargo[c];
			if ( !cargoDef )
				continue;
			if ( Math.RandomFloat( 0.0, 1.0 ) <= Clamp01( cargoDef.Chance ) )
			{
				EntityAI cargo = CreateInCargoSafe( player, cargoDef.ClassName );
				ApplyItemRecursive( cargo, cargoDef, player );
			}
		}
	}
}

static SurviveStartingLoadout BuildDefaultStartingLoadoutTemplate()
{
	SurviveStartingLoadout root = new SurviveStartingLoadout();
	root.ClassName = "";
	root.Chance = 1.0;

	// 空模板：仅保留常用槽位结构，不写死任何物品名。
	// 你现有的 $profile:StartingGearLoadout.json 已经包含旧 init.c 的装备时，不会走到这个模板。
	string slotNames[] = { "Headgear", "Body", "Legs", "Gloves", "Hips", "Mask", "Feet", "Back", "Vest", "Shoulder" };
	foreach ( string slotName : slotNames )
	{
		SurviveLoadoutAttachment slot = new SurviveLoadoutAttachment();
		slot.SlotName = slotName;
		// slot.Items 默认是空数组：你可以在 JSON 里自行填 Items
		root.InventoryAttachments.Insert( slot );
	}

	// root.InventoryCargo 默认空：需要额外物资可在 JSON 里填
	return root;
}

static SurviveStartingLoadout GetSurviveStartingLoadout()
{
	if ( !g_SurviveStartingLoadout )
		g_SurviveStartingLoadout = new SurviveStartingLoadout();

	string path = "$profile:StartingGearLoadout.json";

	if ( !g_SurviveStartingLoadoutLoaded )
	{
		if ( FileExist( path ) )
		{
			JsonFileLoader<SurviveStartingLoadout>.JsonLoadFile( path, g_SurviveStartingLoadout );
		}
		else
		{
			g_SurviveStartingLoadout = BuildDefaultStartingLoadoutTemplate();
			JsonFileLoader<SurviveStartingLoadout>.JsonSaveFile( path, g_SurviveStartingLoadout );
		}

		g_SurviveStartingLoadoutLoaded = true;
	}
	else
	{
		if ( !FileExist( path ) )
			JsonFileLoader<SurviveStartingLoadout>.JsonSaveFile( path, g_SurviveStartingLoadout );
	}

	return g_SurviveStartingLoadout;
}

class CustomMission: MissionServer
{
	void DestroyAttachmentInSlot(PlayerBase player, string slotName)
	{
		if ( !player || slotName == "" )
			return;

		EntityAI attachment = player.FindAttachmentBySlotName( slotName );
		if ( attachment )
			GetGame().ObjectDelete( attachment );
	}

	void SetHealthRange(EntityAI itemEnt, float minHlt, float maxHlt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomFloat( minHlt, maxHlt );
			itemEnt.SetHealth01( "", "", rndHlt );
		}
	}

	void SetRandomHealth(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomFloat( 0.25, 0.65 );
			itemEnt.SetHealth01( "", "", rndHlt );
		}
	}
	
	void SetLowHealth(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomFloat( 0.15, 0.35 );
			itemEnt.SetHealth01( "", "", rndHlt );
		}
	}
	
	void SetQuantity(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomInt( 1, 5 );
			itemEnt.SetQuantity(rndHlt);
		}
	}

	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		Entity playerEnt;
		playerEnt = GetGame().CreatePlayer( identity, characterName, pos, 0, "NONE" );
		Class.CastTo( m_player, playerEnt );

		GetGame().SelectPlayer( identity, m_player );

		return m_player;
	}

	override void StartingEquipSetup(PlayerBase player, bool clothesChosen)
	{
		SurviveStartingLoadout loadout = GetSurviveStartingLoadout();
		ApplyStartingLoadoutToPlayer( player, loadout );
	}
};

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}