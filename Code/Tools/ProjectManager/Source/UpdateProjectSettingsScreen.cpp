/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <UpdateProjectSettingsScreen.h>
#include <PythonBindingsInterface.h>
#include <ProjectManagerDefs.h>
#include <FormImageBrowseEditWidget.h>
#include <FormLineEditWidget.h>
#include <FormComboBoxWidget.h>
#include <AzCore/Utils/Utils.h>

#include <QVBoxLayout>
#include <QLineEdit>
#include <QDir>
#include <QLabel>
#include <QFileInfo>
#include <QPushButton>
#include <QComboBox>

#include <QJsonDocument>
#include <QJsonObject>

// Atom_RPI.Edit
#include <Atom/RPI.Edit/Rendering/RenderingSettingData.h>
#include <Atom/RPI.Edit/Rendering/RenderingSettingUtil.h>

namespace O3DE::ProjectManager
{
    UpdateProjectSettingsScreen::UpdateProjectSettingsScreen(QWidget* parent)
        : ProjectSettingsScreen(parent)
        , m_userChangedPreview(false)
    {
        // Engine combo box
        m_projectEngine = new FormComboBoxWidget(tr("Engine"), {}, this);
        connect(m_projectEngine->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UpdateProjectSettingsScreen::OnProjectEngineUpdated);
        m_verticalLayout->addWidget(m_projectEngine);

        // Project preview browse edit 
        m_projectPreview = new FormImageBrowseEditWidget(tr("Project Preview"), "", this);
        m_projectPreview->lineEdit()->setReadOnly(true);
        connect(m_projectPreview->lineEdit(), &QLineEdit::textChanged, this, &ProjectSettingsScreen::Validate);
        connect(m_projectPreview->lineEdit(), &QLineEdit::textChanged, this, &UpdateProjectSettingsScreen::PreviewPathChanged);
        connect(m_projectPath->lineEdit(), &QLineEdit::textChanged, this, &UpdateProjectSettingsScreen::UpdateProjectPreviewPath);
        m_verticalLayout->addWidget(m_projectPreview);

        QVBoxLayout* previewExtrasLayout = new QVBoxLayout();
        previewExtrasLayout->setAlignment(Qt::AlignTop);
        previewExtrasLayout->setContentsMargins(30, 45, 30, 0);

        QLabel* projectPreviewLabel = new QLabel(tr("Project Preview"));
        previewExtrasLayout->addWidget(projectPreviewLabel);

        // Project preview image
        m_projectPreviewImage = new QLabel(this);
        m_projectPreviewImage->setFixedSize(ProjectPreviewImageWidth, ProjectPreviewImageHeight);
        m_projectPreviewImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        previewExtrasLayout->addWidget(m_projectPreviewImage);

        QLabel* projectPreviewInfoLabel = new QLabel(tr("Select an image (PNG). Minimum %1 x %2 pixels.")
            .arg(QString::number(ProjectPreviewImageWidth), QString::number(ProjectPreviewImageHeight)));
        projectPreviewInfoLabel->setObjectName("projectSmallInfoLabel");
        projectPreviewInfoLabel->setWordWrap(true);
        previewExtrasLayout->addWidget(projectPreviewInfoLabel);

        m_horizontalLayout->addLayout(previewExtrasLayout);

        m_verticalLayout->addSpacing(10);

        // Collapse button
        QHBoxLayout* advancedCollapseLayout = new QHBoxLayout();
        advancedCollapseLayout->setContentsMargins(50, 0, 0, 0);

        m_advancedSettingsCollapseButton = new QPushButton();
        m_advancedSettingsCollapseButton->setCheckable(true);
        m_advancedSettingsCollapseButton->setChecked(true);
        m_advancedSettingsCollapseButton->setFlat(true);
        m_advancedSettingsCollapseButton->setFocusPolicy(Qt::NoFocus);
        m_advancedSettingsCollapseButton->setFixedWidth(s_collapseButtonSize);
        connect(m_advancedSettingsCollapseButton, &QPushButton::clicked, this, &UpdateProjectSettingsScreen::UpdateAdvancedSettingsCollapseState);
        advancedCollapseLayout->addWidget(m_advancedSettingsCollapseButton);

        // Category title
        QLabel* advancedLabel = new QLabel(tr("Advanced Settings"));
        advancedLabel->setObjectName("projectSettingsSectionTitle");
        advancedCollapseLayout->addWidget(advancedLabel);
        m_verticalLayout->addLayout(advancedCollapseLayout);

        m_verticalLayout->addSpacing(5);

        // Everything in the advanced settings widget will be collapsed/uncollapsed
        {
            m_advancedSettingWidget = new QWidget();
            m_verticalLayout->addWidget(m_advancedSettingWidget);

            QVBoxLayout* advancedSettingsLayout = new QVBoxLayout();
            advancedSettingsLayout->setMargin(0);
            advancedSettingsLayout->setAlignment(Qt::AlignTop);
            m_advancedSettingWidget->setLayout(advancedSettingsLayout);

            m_projectId = new FormLineEditWidget(tr("Project ID"), "", this);
            connect(m_projectId->lineEdit(), &QLineEdit::textChanged, this, &UpdateProjectSettingsScreen::OnProjectIdUpdated);
            advancedSettingsLayout->addWidget(m_projectId);

            // AA Settings
            m_aaModeComboBox = new FormComboBoxWidget(tr("Anti-Aliasing Method"), {}, this);
            advancedSettingsLayout->addWidget(m_aaModeComboBox);

            m_multiSampleComboBox = new FormComboBoxWidget(tr("Multi-Sample"), {}, this);
            advancedSettingsLayout->addWidget(m_multiSampleComboBox);
        }

        UpdateAdvancedSettingsCollapseState();
        // Reflect
        AZ::SerializeContext* serializeContext = nullptr;
        EBUS_EVENT_RESULT(serializeContext, AZ::ComponentApplicationBus, GetSerializeContext);
        AZ_Assert(serializeContext, "Serialization context not available");
        if (serializeContext)
        {
            EditorRenderingSettingData::Reflect(serializeContext);
        }
    }


    UpdateProjectSettingsScreen::~UpdateProjectSettingsScreen()
    {
        EditorRenderingSettingData::UnReflect();
    }

    ProjectManagerScreen UpdateProjectSettingsScreen::GetScreenEnum()
    {
        return ProjectManagerScreen::UpdateProjectSettings;
    }

    bool UpdateProjectSettingsScreen::ReadFileToSetComboBox()
    {
        QByteArray jsonArray;
        QString projecPath = m_projectInfo.m_path;
        if (projecPath.isEmpty())
        {
            return false;
        }
        QString filePath = projecPath + REGISTRY_FILENAME;
        QFile file(QDir::toNativeSeparators(filePath));
        if (!file.exists())
        {
            return false;
        }
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        jsonArray = file.readAll();
        file.close();

        QJsonParseError jsonParseError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonArray, &jsonParseError);
        if (jsonDocument.isNull() || jsonParseError.error != QJsonParseError::NoError || !jsonDocument.isObject())
        {
            return false;
        }

        QJsonObject jsonObject = jsonDocument.object();
        QJsonValue engineValue = jsonObject.value("O3DE");
        QJsonValue renderingValue = engineValue.toObject().value("Rendering");
        QJsonObject renderingObject = renderingValue.toObject();
        m_aaModeComboBox->comboBox()->setCurrentText(renderingObject.value("AAMethod").toString());
        m_multiSampleComboBox->comboBox()->setCurrentText(renderingObject.value("MultiSample").toString());

        return true;
    }
    ProjectInfo UpdateProjectSettingsScreen::GetProjectInfo()
    {
        m_projectInfo.m_displayName = m_projectName->lineEdit()->text();
        m_projectInfo.m_version = m_projectVersion->lineEdit()->text();
        m_projectInfo.m_path = m_projectPath->lineEdit()->text();
        m_projectInfo.m_id = m_projectId->lineEdit()->text();

        if (m_userChangedPreview)
        {
            m_projectInfo.m_iconPath = ProjectPreviewImagePath;
            m_projectInfo.m_newPreviewImagePath = m_projectPreview->lineEdit()->text();
        }
        return m_projectInfo;
    }

    void UpdateProjectSettingsScreen::SetProjectInfo(const ProjectInfo& projectInfo)
    {
        m_projectInfo = projectInfo;

        m_projectName->lineEdit()->setText(projectInfo.GetProjectDisplayName());
        m_projectVersion->lineEdit()->setText(projectInfo.m_version);
        m_projectPath->lineEdit()->setText(projectInfo.m_path);
        m_projectId->lineEdit()->setText(projectInfo.m_id);

        UpdateProjectPreviewPath();

        QComboBox* combobox = m_projectEngine->comboBox();
        combobox->clear();

        // we use engine path which is unique instead of engine name which may not be
        EngineInfo assignedEngine;
        if(auto result = PythonBindingsInterface::Get()->GetProjectEngine(projectInfo.m_path); result)
        {
            assignedEngine = result.TakeValue();
        }

        // handle case where user may not want to set the engine name (engine-centric) 
        int index = 0;
        int selectedIndex = -1;
        if (projectInfo.m_engineName.isEmpty() && !assignedEngine.m_path.isEmpty())
        {
            combobox->addItem(
                QString("(no engine specified) %1 %2 (%3)").
                    arg(assignedEngine.m_name,assignedEngine.m_version, assignedEngine.m_path),
                    QStringList{ assignedEngine.m_path, "" });
            selectedIndex = index;
            index++;
        }
        // handle case when project uses an engine that isn't registered
        else if (!projectInfo.m_engineName.isEmpty() && assignedEngine.m_path.isEmpty())
        {
            combobox->addItem(QString("%1 (not registered)").arg(projectInfo.m_engineName), QStringList{ "", projectInfo.m_engineName });
            selectedIndex = index;
            index++;
        }

    
        if (auto result = PythonBindingsInterface::Get()->GetAllEngineInfos(); result)
        {
            for (auto engineInfo : result.GetValue<QVector<EngineInfo>>())
            {
                if (!engineInfo.m_name.isEmpty())
                {
                    const bool useDisplayVersion = !engineInfo.m_displayVersion.isEmpty() &&
                                                    engineInfo.m_displayVersion != "00.00" &&
                                                    engineInfo.m_displayVersion != "0.1.0.0";
                    const auto engineVersion = useDisplayVersion ? engineInfo.m_displayVersion : engineInfo.m_version;

                    combobox->addItem(
                        QString("%1 %2 (%3)").arg(engineInfo.m_name, engineVersion, engineInfo.m_path),
                        QStringList{ engineInfo.m_path, engineInfo.m_name });

                    if (selectedIndex == -1 && !assignedEngine.m_path.isEmpty() && QDir(assignedEngine.m_path) == QDir(engineInfo.m_path))
                    {
                        selectedIndex = index;
                    }
                    index++;
                }
            }
        }

        if (selectedIndex != -1)
        {
            combobox->setCurrentIndex(selectedIndex);
        }

        combobox->setVisible(combobox->count() > 0);

         // Anti-Aliasing
        combobox = m_aaModeComboBox->comboBox();
        combobox->clear();
        combobox->addItems(QStringList{ tr("MSAA"), tr("SMAA"), tr("TAA")});
        combobox->setCurrentIndex(0);

        combobox = m_multiSampleComboBox->comboBox();
        combobox->clear();
        combobox->addItems(QStringList{ tr("1"), tr("2"), tr("4")});
        combobox->setCurrentIndex(0);
        ReadFileToSetComboBox();
    }

    void UpdateProjectSettingsScreen::UpdateProjectPreviewPath()
    {
        if (!m_userChangedPreview)
        {
            m_projectPreview->lineEdit()->setText(QDir(m_projectPath->lineEdit()->text()).filePath(m_projectInfo.m_iconPath));
            // Setting the text sets m_userChangedPreview to true
            // Set it back to false because it should only be true when changed by user
            m_userChangedPreview = false;
        }
    }

    AZ::Outcome<void, QString> UpdateProjectSettingsScreen::Validate() const
    {
        if (!(ValidateProjectPreview() && ValidateProjectId()))
        {
            // Returning empty string to use the default error message
            return AZ::Failure<QString>("");
        }

        return ProjectSettingsScreen::Validate();
    }

    void UpdateProjectSettingsScreen::ResetProjectPreviewPath()
    {
        m_userChangedPreview = false;
        UpdateProjectPreviewPath();
    }

    void UpdateProjectSettingsScreen::PreviewPathChanged()
    {
        m_userChangedPreview = true;

        // Update with latest image
        m_projectPreviewImage->setPixmap(
            QPixmap(m_projectPreview->lineEdit()->text()).scaled(m_projectPreviewImage->size(), Qt::KeepAspectRatioByExpanding));
    }

    void UpdateProjectSettingsScreen::OnProjectIdUpdated()
    {
        ValidateProjectId();
    }

    void UpdateProjectSettingsScreen::OnProjectEngineUpdated(int index)
    {
        auto value = m_projectEngine->comboBox()->itemData(index).value<QStringList>();
        if (value.count() == 2)
        {
            m_projectInfo.m_enginePath = value[0];
            m_projectInfo.m_engineName = value[1];
        }
    } 


    bool UpdateProjectSettingsScreen::ValidateProjectPath() const
    {
        bool projectPathIsValid = true;
        QDir path(m_projectPath->lineEdit()->text());
        if (!path.isAbsolute())
        {
            projectPathIsValid = false;
            m_projectPath->setErrorLabelText(tr("Please provide an absolute path for the project location."));
        }

        m_projectPath->setErrorLabelVisible(!projectPathIsValid);
        return projectPathIsValid;
    }

    bool UpdateProjectSettingsScreen::ValidateProjectPreview() const
    {
        bool projectPreviewIsValid = true;

        if (m_projectPreview->lineEdit()->text().isEmpty())
        {
            projectPreviewIsValid = false;
            m_projectPreview->setErrorLabelText(tr("Please select a file."));
        }
        else
        {
            if (m_userChangedPreview)
            {
                QFileInfo previewFile(m_projectPreview->lineEdit()->text());
                if (!previewFile.exists() || !previewFile.isFile())
                {
                    projectPreviewIsValid = false;
                    m_projectPreview->setErrorLabelText(tr("Please select a valid png file."));
                }
                else
                {
                    QString fileType = previewFile.completeSuffix().toLower();
                    if (fileType != "png")
                    {
                        projectPreviewIsValid = false;
                        m_projectPreview->setErrorLabelText(tr("Please select a png image."));
                    }
                }
            }
        }

        m_projectPreview->setErrorLabelVisible(!projectPreviewIsValid);
        return projectPreviewIsValid;
    }

    bool UpdateProjectSettingsScreen::ValidateProjectId() const
    {
        bool projectIdIsValid = true;
        if (m_projectId->lineEdit()->text().isEmpty())
        {
            projectIdIsValid = false;
            m_projectId->setErrorLabelText(tr("Project ID cannot be empty."));
        }

        m_projectId->setErrorLabelVisible(!projectIdIsValid);
        return projectIdIsValid;
    }

    void UpdateProjectSettingsScreen::UpdateAdvancedSettingsCollapseState()
    {
        if (m_advancedSettingsCollapseButton->isChecked())
        {
            m_advancedSettingsCollapseButton->setIcon(QIcon(":/ArrowDownLine.svg"));
            m_advancedSettingWidget->hide();
        }
        else
        {
            m_advancedSettingsCollapseButton->setIcon(QIcon(":/ArrowUpLine.svg"));
            m_advancedSettingWidget->show();
        }
    }

    int UpdateProjectSettingsScreen::GetAAMethodIndex()
    {
        return m_aaModeComboBox->comboBox()->currentIndex();
    }

    int UpdateProjectSettingsScreen::GetMultiSampleNum()
    {
        int multiNum = m_multiSampleComboBox->comboBox()->currentIndex() + 1;
        multiNum = multiNum <= 2 ? multiNum : 4;

        return multiNum;
    }

    int UpdateProjectSettingsScreen::GetMultiSampleIndex()
    {
        return m_multiSampleComboBox->comboBox()->currentIndex();
    }

    void UpdateProjectSettingsScreen::SaveJsonToFile()
    {
        QJsonObject engineObject;
        QJsonObject renderingObject;
        QJsonObject renderingMemberObject;

        renderingMemberObject.insert("AAMethod", m_aaModeComboBox->comboBox()->currentText());
        renderingMemberObject.insert("MultiSample", m_multiSampleComboBox->comboBox()->currentText());
        renderingObject.insert("Rendering", renderingMemberObject);
        engineObject.insert("O3DE", renderingObject);

        QJsonDocument jsonDocument(engineObject);
        QByteArray jsonArray = jsonDocument.toJson(QJsonDocument::Indented);
        QString projecPath = m_projectPath->lineEdit()->text();
        QString filePath = projecPath + REGISTRY_FILENAME;
        QFile file(QDir::toNativeSeparators(filePath));
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            return;
        }
        file.write(jsonArray);
        file.close();
        {
            ProjectInfo projectInfo;
            AZ::IO::FixedMaxPath fixedProjectPath = m_projectPath->lineEdit()->text().toStdString().c_str();
            auto projectResult = PythonBindingsInterface::Get()->GetProject(fixedProjectPath.c_str());
            if (projectResult.IsSuccess())
            {
                projectInfo = projectResult.GetValue();
            }

            EngineInfo assignedEngine;
            if (auto result = PythonBindingsInterface::Get()->GetProjectEngine(projectInfo.m_path); result)
            {
                assignedEngine = result.TakeValue();
            }
            
            AZStd::string rootPath;
            AZ::StringFunc::Path::ConstructFull(
                assignedEngine.m_path.toStdString().c_str(), "Gems/Atom/Feature/Common/Assets/Passes/MainRenderPipeline.azasset", rootPath);

            EditorRenderingSettingData renderingSettingData;
            RenderingSetting::LoadFromFile(rootPath, renderingSettingData);

            AZStd::string renderingAAMethod = m_aaModeComboBox->comboBox()->currentText().toStdString().c_str();
            int multiSampleNum = GetMultiSampleNum();
            renderingSettingData.m_renderPipelineDescriptor.m_defaultAAMethod = (renderingAAMethod == "MSAA" ? "MSAA" : renderingAAMethod);
            renderingSettingData.m_renderPipelineDescriptor.m_renderSettings.m_multisampleState.m_samples = static_cast<uint16_t>(multiSampleNum);
            RenderingSetting::SaveToFile(rootPath, renderingSettingData);
        }
    }
} // namespace O3DE::ProjectManager
