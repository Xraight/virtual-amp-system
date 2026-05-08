#pragma once
#include <JuceHeader.h>
#include "sqlite3.h"

class PresetDatabase
{
public:
    PresetDatabase()
    {
        auto dbPath = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory)
            .getChildFile("NoEraClean")
            .getChildFile("presets.db");

        dbPath.getParentDirectory().createDirectory();

        int rc = sqlite3_open(dbPath.getFullPathName().toStdString().c_str(), &db);
        if (rc != SQLITE_OK)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "ERROR DB",
                "No se pudo abrir: " + juce::String(sqlite3_errmsg(db)));
        }
        else
        {
            createTables();

        }
    }

    ~PresetDatabase()
    {
        if (db) sqlite3_close(db);
    }

    // ===================================================
    // STRUCT PRESET — todos los parámetros
    // ===================================================
    struct Preset
    {
        int id = 0;
        juce::String nombre;

        // Amp
        float gain = 5.0f, bass = 5.0f, mid = 5.0f;
        float treble = 5.0f, presence = 5.0f, master = 5.0f;

        // Input / Output
        float input = 0.0f, output = 0.0f;

        // Cabinet
        float lowcut = 10.0f, highcut = 20000.0f;

        // Overdrive
        float odDrive = 0.5f, odTone = 0.5f, odLevel = 0.5f;

        // Chorus
        float choRate = 0.3f, choDepth = 0.3f, choLevel = 0.5f;

        // Delay
        float delTime = 0.3f, delFb = 0.4f, delMix = 0.3f;

        // EQ 6 bandas
        float eq1 = 0.0f, eq2 = 0.0f, eq3 = 0.0f;
        float eq4 = 0.0f, eq5 = 0.0f, eq6 = 0.0f;

        juce::String fecha;
    };

    // ===================================================
    // GUARDAR PRESET — todos los parámetros
    // ===================================================
    void savePreset(const Preset& p)
    {
        // Usar prepared statement para evitar SQL injection y problemas con nombres
        const char* sql =
            "INSERT INTO presets "
            "(nombre, gain, bass, mid, treble, presence, master, "
            " input_gain, output_gain, lowcut, highcut, "
            " od_drive, od_tone, od_level, "
            " cho_rate, cho_depth, cho_level, "
            " del_time, del_fb, del_mix, "
            " eq1, eq2, eq3, eq4, eq5, eq6, fecha) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,datetime('now'));";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            DBG("Error preparando INSERT: " << sqlite3_errmsg(db));
            return;
        }

        sqlite3_bind_text(stmt, 1, p.nombre.toStdString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 2, p.gain);
        sqlite3_bind_double(stmt, 3, p.bass);
        sqlite3_bind_double(stmt, 4, p.mid);
        sqlite3_bind_double(stmt, 5, p.treble);
        sqlite3_bind_double(stmt, 6, p.presence);
        sqlite3_bind_double(stmt, 7, p.master);
        sqlite3_bind_double(stmt, 8, p.input);
        sqlite3_bind_double(stmt, 9, p.output);
        sqlite3_bind_double(stmt, 10, p.lowcut);
        sqlite3_bind_double(stmt, 11, p.highcut);
        sqlite3_bind_double(stmt, 12, p.odDrive);
        sqlite3_bind_double(stmt, 13, p.odTone);
        sqlite3_bind_double(stmt, 14, p.odLevel);
        sqlite3_bind_double(stmt, 15, p.choRate);
        sqlite3_bind_double(stmt, 16, p.choDepth);
        sqlite3_bind_double(stmt, 17, p.choLevel);
        sqlite3_bind_double(stmt, 18, p.delTime);
        sqlite3_bind_double(stmt, 19, p.delFb);
        sqlite3_bind_double(stmt, 20, p.delMix);
        sqlite3_bind_double(stmt, 21, p.eq1);
        sqlite3_bind_double(stmt, 22, p.eq2);
        sqlite3_bind_double(stmt, 23, p.eq3);
        sqlite3_bind_double(stmt, 24, p.eq4);
        sqlite3_bind_double(stmt, 25, p.eq5);
        sqlite3_bind_double(stmt, 26, p.eq6);

        bool guardadoOk = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);

        // Si fue exitoso, NO mostramos popup aquí — el editor lo maneja
    }

    // ===================================================
    // ACTUALIZAR PRESET EXISTENTE
    // ===================================================
    void updatePreset(const Preset& p)
    {
        const char* sql =
            "UPDATE presets SET "
            "nombre=?, gain=?, bass=?, mid=?, treble=?, presence=?, master=?, "
            "input_gain=?, output_gain=?, lowcut=?, highcut=?, "
            "od_drive=?, od_tone=?, od_level=?, "
            "cho_rate=?, cho_depth=?, cho_level=?, "
            "del_time=?, del_fb=?, del_mix=?, "
            "eq1=?, eq2=?, eq3=?, eq4=?, eq5=?, eq6=?, "
            "fecha=datetime('now') "
            "WHERE id=?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            DBG("Error preparando UPDATE: " << sqlite3_errmsg(db));
            return;
        }

        sqlite3_bind_text(stmt, 1, p.nombre.toStdString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 2, p.gain);
        sqlite3_bind_double(stmt, 3, p.bass);
        sqlite3_bind_double(stmt, 4, p.mid);
        sqlite3_bind_double(stmt, 5, p.treble);
        sqlite3_bind_double(stmt, 6, p.presence);
        sqlite3_bind_double(stmt, 7, p.master);
        sqlite3_bind_double(stmt, 8, p.input);
        sqlite3_bind_double(stmt, 9, p.output);
        sqlite3_bind_double(stmt, 10, p.lowcut);
        sqlite3_bind_double(stmt, 11, p.highcut);
        sqlite3_bind_double(stmt, 12, p.odDrive);
        sqlite3_bind_double(stmt, 13, p.odTone);
        sqlite3_bind_double(stmt, 14, p.odLevel);
        sqlite3_bind_double(stmt, 15, p.choRate);
        sqlite3_bind_double(stmt, 16, p.choDepth);
        sqlite3_bind_double(stmt, 17, p.choLevel);
        sqlite3_bind_double(stmt, 18, p.delTime);
        sqlite3_bind_double(stmt, 19, p.delFb);
        sqlite3_bind_double(stmt, 20, p.delMix);
        sqlite3_bind_double(stmt, 21, p.eq1);
        sqlite3_bind_double(stmt, 22, p.eq2);
        sqlite3_bind_double(stmt, 23, p.eq3);
        sqlite3_bind_double(stmt, 24, p.eq4);
        sqlite3_bind_double(stmt, 25, p.eq5);
        sqlite3_bind_double(stmt, 26, p.eq6);
        sqlite3_bind_int(stmt, 27, p.id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            DBG("Error ejecutando UPDATE: " << sqlite3_errmsg(db));
        else
            DBG("Preset actualizado ID: " << p.id);

        sqlite3_finalize(stmt);
    }

    // ===================================================
    // OBTENER TODOS LOS PRESETS
    // ===================================================
    std::vector<Preset> getAllPresets()
    {
        std::vector<Preset> presets;
        sqlite3_stmt* stmt;

        const char* sql =
            "SELECT id, nombre, gain, bass, mid, treble, presence, master, "
            "input_gain, output_gain, lowcut, highcut, "
            "od_drive, od_tone, od_level, "
            "cho_rate, cho_depth, cho_level, "
            "del_time, del_fb, del_mix, "
            "eq1, eq2, eq3, eq4, eq5, eq6, fecha "
            "FROM presets ORDER BY id DESC;";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            DBG("Error en SELECT: " << sqlite3_errmsg(db));
            return presets;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Preset p;
            p.id = sqlite3_column_int(stmt, 0);
            p.nombre = juce::String((const char*)sqlite3_column_text(stmt, 1));
            p.gain = (float)sqlite3_column_double(stmt, 2);
            p.bass = (float)sqlite3_column_double(stmt, 3);
            p.mid = (float)sqlite3_column_double(stmt, 4);
            p.treble = (float)sqlite3_column_double(stmt, 5);
            p.presence = (float)sqlite3_column_double(stmt, 6);
            p.master = (float)sqlite3_column_double(stmt, 7);
            p.input = (float)sqlite3_column_double(stmt, 8);
            p.output = (float)sqlite3_column_double(stmt, 9);
            p.lowcut = (float)sqlite3_column_double(stmt, 10);
            p.highcut = (float)sqlite3_column_double(stmt, 11);
            p.odDrive = (float)sqlite3_column_double(stmt, 12);
            p.odTone = (float)sqlite3_column_double(stmt, 13);
            p.odLevel = (float)sqlite3_column_double(stmt, 14);
            p.choRate = (float)sqlite3_column_double(stmt, 15);
            p.choDepth = (float)sqlite3_column_double(stmt, 16);
            p.choLevel = (float)sqlite3_column_double(stmt, 17);
            p.delTime = (float)sqlite3_column_double(stmt, 18);
            p.delFb = (float)sqlite3_column_double(stmt, 19);
            p.delMix = (float)sqlite3_column_double(stmt, 20);
            p.eq1 = (float)sqlite3_column_double(stmt, 21);
            p.eq2 = (float)sqlite3_column_double(stmt, 22);
            p.eq3 = (float)sqlite3_column_double(stmt, 23);
            p.eq4 = (float)sqlite3_column_double(stmt, 24);
            p.eq5 = (float)sqlite3_column_double(stmt, 25);
            p.eq6 = (float)sqlite3_column_double(stmt, 26);
            p.fecha = juce::String((const char*)sqlite3_column_text(stmt, 27));
            presets.push_back(p);
        }

        sqlite3_finalize(stmt);
        return presets;
    }

    // ===================================================
    // OBTENER UN PRESET POR ID
    // ===================================================
    Preset getPresetByID(int id)
    {
        Preset p;
        sqlite3_stmt* stmt;

        const char* sql =
            "SELECT id, nombre, gain, bass, mid, treble, presence, master, "
            "input_gain, output_gain, lowcut, highcut, "
            "od_drive, od_tone, od_level, "
            "cho_rate, cho_depth, cho_level, "
            "del_time, del_fb, del_mix, "
            "eq1, eq2, eq3, eq4, eq5, eq6, fecha "
            "FROM presets WHERE id=?;";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return p;

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            p.id = sqlite3_column_int(stmt, 0);
            p.nombre = juce::String((const char*)sqlite3_column_text(stmt, 1));
            p.gain = (float)sqlite3_column_double(stmt, 2);
            p.bass = (float)sqlite3_column_double(stmt, 3);
            p.mid = (float)sqlite3_column_double(stmt, 4);
            p.treble = (float)sqlite3_column_double(stmt, 5);
            p.presence = (float)sqlite3_column_double(stmt, 6);
            p.master = (float)sqlite3_column_double(stmt, 7);
            p.input = (float)sqlite3_column_double(stmt, 8);
            p.output = (float)sqlite3_column_double(stmt, 9);
            p.lowcut = (float)sqlite3_column_double(stmt, 10);
            p.highcut = (float)sqlite3_column_double(stmt, 11);
            p.odDrive = (float)sqlite3_column_double(stmt, 12);
            p.odTone = (float)sqlite3_column_double(stmt, 13);
            p.odLevel = (float)sqlite3_column_double(stmt, 14);
            p.choRate = (float)sqlite3_column_double(stmt, 15);
            p.choDepth = (float)sqlite3_column_double(stmt, 16);
            p.choLevel = (float)sqlite3_column_double(stmt, 17);
            p.delTime = (float)sqlite3_column_double(stmt, 18);
            p.delFb = (float)sqlite3_column_double(stmt, 19);
            p.delMix = (float)sqlite3_column_double(stmt, 20);
            p.eq1 = (float)sqlite3_column_double(stmt, 21);
            p.eq2 = (float)sqlite3_column_double(stmt, 22);
            p.eq3 = (float)sqlite3_column_double(stmt, 23);
            p.eq4 = (float)sqlite3_column_double(stmt, 24);
            p.eq5 = (float)sqlite3_column_double(stmt, 25);
            p.eq6 = (float)sqlite3_column_double(stmt, 26);
            p.fecha = juce::String((const char*)sqlite3_column_text(stmt, 27));
        }

        sqlite3_finalize(stmt);
        return p;
    }

    // ===================================================
    // BORRAR PRESET
    // ===================================================
    void deletePreset(int id)
    {
        sqlite3_stmt* stmt;
        const char* sql = "DELETE FROM presets WHERE id=?;";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return;

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            DBG("Error borrando preset: " << sqlite3_errmsg(db));
        else
            DBG("Preset borrado ID: " << id);

        sqlite3_finalize(stmt);
    }

private:
    sqlite3* db = nullptr;

    void createTables()
    {
        const char* sql =
            "CREATE TABLE IF NOT EXISTS presets ("
            "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre      TEXT    NOT NULL,"
            "gain        REAL    DEFAULT 5.0,"
            "bass        REAL    DEFAULT 5.0,"
            "mid         REAL    DEFAULT 5.0,"
            "treble      REAL    DEFAULT 5.0,"
            "presence    REAL    DEFAULT 5.0,"
            "master      REAL    DEFAULT 5.0,"
            "input_gain  REAL    DEFAULT 0.0,"
            "output_gain REAL    DEFAULT 0.0,"
            "lowcut      REAL    DEFAULT 10.0,"
            "highcut     REAL    DEFAULT 20000.0,"
            "od_drive    REAL    DEFAULT 0.5,"
            "od_tone     REAL    DEFAULT 0.5,"
            "od_level    REAL    DEFAULT 0.5,"
            "cho_rate    REAL    DEFAULT 0.3,"
            "cho_depth   REAL    DEFAULT 0.3,"
            "cho_level   REAL    DEFAULT 0.5,"
            "del_time    REAL    DEFAULT 0.3,"
            "del_fb      REAL    DEFAULT 0.4,"
            "del_mix     REAL    DEFAULT 0.3,"
            "eq1         REAL    DEFAULT 0.0,"
            "eq2         REAL    DEFAULT 0.0,"
            "eq3         REAL    DEFAULT 0.0,"
            "eq4         REAL    DEFAULT 0.0,"
            "eq5         REAL    DEFAULT 0.0,"
            "eq6         REAL    DEFAULT 0.0,"
            "fecha       TEXT);";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            DBG("Error creando tabla: " << errMsg);
            sqlite3_free(errMsg);
        }
        else
        {
            DBG("Tabla presets lista.");
            // Migración: agregar columnas nuevas si la tabla ya existía sin ellas
            migrateTable();
            insertDefaultPresets();

        }
    }

    // Agrega columnas que puedan faltar en DBs creadas con versiones anteriores
    void migrateTable()
    {
        auto addColumnIfMissing = [this](const char* col, const char* colType, const char* def)
            {
                juce::String sql = juce::String("ALTER TABLE presets ADD COLUMN ") +
                    col + " " + colType + " DEFAULT " + def + ";";
                sqlite3_exec(db, sql.toStdString().c_str(), nullptr, nullptr, nullptr);
                // Ignoramos el error si la columna ya existe
            };

        addColumnIfMissing("input_gain", "REAL", "0.0");
        addColumnIfMissing("output_gain", "REAL", "0.0");
        addColumnIfMissing("lowcut", "REAL", "10.0");
        addColumnIfMissing("highcut", "REAL", "20000.0");
        addColumnIfMissing("od_tone", "REAL", "0.5");
        addColumnIfMissing("od_level", "REAL", "0.5");
        addColumnIfMissing("cho_depth", "REAL", "0.3");
        addColumnIfMissing("cho_level", "REAL", "0.5");
        addColumnIfMissing("del_fb", "REAL", "0.4");
        addColumnIfMissing("del_mix", "REAL", "0.3");
    }

    void insertDefaultPresets()
    {
        sqlite3_stmt* check;
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM presets;", -1, &check, nullptr);
        int count = 0;
        if (sqlite3_step(check) == SQLITE_ROW)
            count = sqlite3_column_int(check, 0);
        sqlite3_finalize(check);

        if (count > 0) return;

        const char* nombre = "Clean Default";
        sqlite3_stmt* stmt;
        const char* sql =
            "INSERT INTO presets "
            "(nombre, gain, bass, mid, treble, presence, master, "
            " input_gain, output_gain, lowcut, highcut, "
            " od_drive, od_tone, od_level, "
            " cho_rate, cho_depth, cho_level, "
            " del_time, del_fb, del_mix, "
            " eq1, eq2, eq3, eq4, eq5, eq6, fecha) "
            "VALUES (?,5.0,5.0,5.0,5.0,5.0,5.0,"
            "0.0,0.0,10.0,20000.0,"
            "0.5,0.5,0.5,"
            "0.3,0.3,0.5,"
            "0.3,0.4,0.3,"
            "0.0,0.0,0.0,0.0,0.0,0.0,datetime('now'));";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, nombre, -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
};