#include "quattercam.h"
#include "effectmaster.h"
#include "board.h"
#include "piece.h"

Piece::Piece(Attributes attributes): Object(MC->GetContext()),
    attributes_{attributes},
    state_{PieceState::FREE }
{
    rootNode_ = MC->world_.scene_->CreateChild("Piece"+GetCodon(4));
    rootNode_->SetRotation(Quaternion(Random(360.0f), Vector3::UP));

    StaticModel* pieceModel{rootNode_->CreateComponent<StaticModel>()};
    pieceModel->SetCastShadows(true);
    pieceModel->SetModel(MC->GetModel("Piece_"+GetCodon(3)));
    if (attributes[3]){
        pieceModel->SetMaterial(MC->GetMaterial("Wood_light"));
    }
    else pieceModel->SetMaterial(MC->GetMaterial("Wood_dark"));

    outlineModel_ = rootNode_->CreateComponent<StaticModel>();
    outlineModel_->SetCastShadows(false);
    outlineModel_->SetModel(MC->GetModel("Piece_"+GetCodon(2)+"_outline"));
    outlineModel_->SetMaterial(MC->GetMaterial("Glow")->Clone());
    outlineModel_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f));
    outlineModel_->SetEnabled(false);

    Node* lightNode{rootNode_->CreateChild("Light")};
    lightNode->SetPosition(Vector3::UP * 0.5f);
    light_ = lightNode->CreateComponent<Light>();
    light_->SetColor(Color(0.0f, 0.8f, 0.5f));
    light_->SetBrightness(0.0f);
    light_->SetRange(3.0f);
}
void Piece::Reset()
{
    rootNode_->SetParent(MC->world_.scene_);

    if (MC->GetSelectedPiece() == this){
        MC->DeselectPiece();
    } else Deselect();

    if (state_ != PieceState::FREE){
        state_ = PieceState::FREE;
        FX->ArchTo(rootNode_,
                   MC->AttributesToPosition(ToInt()),
                   Quaternion(Random(360.0f), Vector3::UP),
                   attributes_[0] ? 2.0f : 1.3f + attributes_[1] ? 0.5f : 1.0f + Random(0.23f),
                   RESET_DURATION, true);
    }
}

String Piece::GetCodon(int length) const
{
    if (length > static_cast<int>(attributes_.size()) || length < 1)
        length = static_cast<int>(attributes_.size());

    String codon{};

        codon += attributes_[0] ? "T" : "S"; //Tall  : Short
    if (length > 1)
        codon += attributes_[1] ? "R" : "S"; //Round : Square
    if (length > 2)
        codon += attributes_[2] ? "H" : "S"; //Hole  : Solid
    if (length == NUM_ATTRIBUTES)
        codon += attributes_[3] ? "L" : "D"; //Light : Dark

    return codon;
}

void Piece::Select()
{
    if ((MC->GetGameState() == GameState::PLAYER1PICKS ||
         MC->GetGameState() == GameState::PLAYER2PICKS))
    {
        outlineModel_->SetEnabled(true);
        if (state_ == PieceState::FREE){
            state_ = PieceState::SELECTED;
            FX->FadeTo(outlineModel_->GetMaterial(),
                                      COLOR_GLOW);
            FX->FadeTo(light_, 0.666f);
        }
    }
}
void Piece::Deselect()
{
    if (state_ == PieceState::SELECTED){
        state_ = PieceState::FREE;
        FX->FadeOut(outlineModel_->GetMaterial());
        FX->FadeOut(light_);
    }
}
void Piece::Pick()
{
    if (state_ == PieceState::SELECTED){
        state_ = PieceState::PICKED;
        if (MC->GetGameState() == GameState::PLAYER1PICKS)
            rootNode_->SetParent(CAMERA->GetPocket(false));
        if (MC->GetGameState() == GameState::PLAYER2PICKS)
            rootNode_->SetParent(CAMERA->GetPocket(true));

        FX->ArchTo(rootNode_, Vector3::DOWN, Quaternion(10.0f, Vector3(1.0f, 0.0f, 0.5f)), 1.0f, 0.8f);

        FX->FadeOut(outlineModel_->GetMaterial());
        FX->FadeOut(light_);
    }
}
void Piece::Put(Vector3 position)
{
    if (state_ == PieceState::PICKED){
        state_ = PieceState::PUT;

        rootNode_->SetParent(MC->world_.scene_);

        FX->ArchTo(rootNode_, position, Quaternion(Random(-13.0f, 13.0f), Vector3::UP), 2.3f, 0.42f);
    }
}
